////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Christoph Uhde
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef ARANGO_CXX_DRIVER_VST_CONNECTION_H
#define ARANGO_CXX_DRIVER_VST_CONNECTION_H 1

#include <atomic>
#include <mutex>
#include <map>
#include <deque>
#include <condition_variable>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/deadline_timer.hpp>

#include <fuerte/connection_interface.h>
#include <fuerte/helper.h>
#include <fuerte/loop.h>
#include <fuerte/vst.h>

// naming in this file will be closer to asio for internal functions and types
// functions that are exposed to other classes follow ArangoDB conding conventions

namespace arangodb { namespace fuerte { inline namespace v1 {

class Loop;

namespace vst {

class VstConnection : public std::enable_shared_from_this<VstConnection>, public ConnectionInterface {

// Connection object that handles sending and receiving of Velocystream
// Messages.
//
// A VstConnection tries to create a connection to one of the given peers on a
// a socket. Then it tries to do an asyncronous ssl handshake. When the
// handshake is done it starts the async read loop on the socket. And tries a
// first write.
//
// The startRead function uses the function asyncReadCallback as handler for the
// async_read on the socket. This handler calls start Read as soon it has taken
// all relevant data from the socket. so The loop runs until the vstConnection
// is stopped.
//
// startWrite will be Triggered after establishing a connection or when new
// data has been queued for sending. Then startWrite and handleWrite call each
// other in the same way as startRead / asyncReadCallback but stop doing so as soon as
// the writeQueue is empty.
//

public:
  explicit VstConnection(EventLoopService& eventLoopService, detail::ConnectionConfiguration const&);

public:
  // this function prepares the request for sending
  // by creating a RequestItem and setting:
  //  - a messageid
  //  - the buffer to be send
  // this item is then moved to the request queue
  // and a write action is triggerd when there is
  // no other write in progress
  MessageID sendRequest(std::unique_ptr<Request>
                       ,OnErrorCallback
                       ,OnSuccessCallback) override;

private:
  // SOCKET HANDLING /////////////////////////////////////////////////////////
  void initSocket();
  void shutdownSocket();
  void shutdownConnection();
  void restartConnection();

  virtual void start() override { initSocket(); }
  virtual void restart() override { initSocket(); }
  virtual size_t requestsLeft() override;

  //handler to be posted to loop
  //this handler call their handle counterpart on completion

  // establishes connection and initiates handshake
  void startConnect(boost::asio::ip::tcp::resolver::iterator);
  void asyncConnectCallback(boost::system::error_code const& ec, boost::asio::ip::tcp::resolver::iterator);

  // start intiating an SSL connection (on top of an established TCP socket)
  void startSSLHandshake();

  // socket connection is up (with optional SSL), now initiate the VST protocol.
  void finishInitialization();

  // activate the receiver loop (if needed)
  inline void startReading() {
    if (!_reading.exchange(true)) readNextBytes(_connectionState);
  }
  // reads data from socket with boost::asio::async_read
  void readNextBytes(int initialConnectionState);
  // handler for boost::asio::async_read that extracs chunks form the network
  // takes complete chunks form the socket and starts a new read action. After
  // triggering the next read it processes the received data.
  void asyncReadCallback(boost::system::error_code const&, std::size_t transferred, int initialConnectionState);
  // Process the given incoming chunk.
  void processChunk(ChunkHeader &chunk);
  // Create a response object for given RequestItem & received response buffer.
  std::unique_ptr<Response> createResponse(RequestItem& item, std::unique_ptr<VBuffer>& responseBuffer);

  // activate the sender loop (if needed)
  inline void startSending() {
    if (!_sending.exchange(true)) sendNextRequest(_connectionState);
  }
  // writes data from task queue to network using boost::asio::async_write
  void sendNextRequest(int initialConnectionState);
  // handler for boost::asio::async_wirte that calls startWrite as long as there is new data
  void asyncWriteCallback(boost::system::error_code const&, std::size_t transferred, std::shared_ptr<RequestItem>, int initialConnectionState);

private:
  VSTVersion _vstVersion;
  // TODO FIXME -- fix alignment when done so mutexes are not on the same cacheline etc
  detail::ConnectionConfiguration _configuration;
  ::std::atomic_uint_least64_t _messageID;
  // socket
  std::shared_ptr<::boost::asio::io_service> _ioService;
  ::std::shared_ptr<::boost::asio::ip::tcp::socket> _socket;
  ::boost::asio::ssl::context _context;
  ::std::shared_ptr<::boost::asio::ssl::stream<::boost::asio::ip::tcp::socket&>> _sslSocket;
  ::boost::asio::ip::tcp::resolver::iterator _endpoints;
  ::boost::asio::ip::tcp::endpoint _peer;
  ::boost::asio::deadline_timer _deadline;
  // reset
  std::atomic_bool _connected;
  std::atomic_int _connectionState; // Send/Receive loop will stop if this changes
  std::atomic_bool _reading;
  std::atomic_bool _sending;
  //queues
  ::boost::asio::streambuf _receiveBuffer; // async read can not run concurrent

  // SendQueue encapsulates a thread safe queue containing RequestItem's that 
  // need sending to the server.
  class SendQueue {
   public:
     // add the given item to the end of the queue.
    void add(std::shared_ptr<RequestItem>& item) {
      std::lock_guard<std::mutex> lockQueue(_mutex);
      _queue.push_back(item);
    }

    // front returns the first item in the queue.
    // If the queue is empty, NULL is returned.
    std::shared_ptr<RequestItem> front() {
      std::lock_guard<std::mutex> sendQueueLock(_mutex);
      if (_queue.empty()) {
        return std::shared_ptr<RequestItem>();
      }
      return _queue.front();
    }

    // removeFirst removes the first entry of the queue.
    void removeFirst() {
      std::lock_guard<std::mutex> sendQueueLock(_mutex);
      _queue.pop_front();
    }

    // size returns the number of elements in the queue.
    size_t size() {
      std::lock_guard<std::mutex> lockQueue(_mutex);
      return _queue.size();
    }

    // empty returns true when there are no elements in the queue, false otherwise.
    bool empty(bool unlocked = false) {
      if (unlocked) {
        return _queue.empty();
      } else {
        std::lock_guard<std::mutex> lockQueue(_mutex);
        return _queue.empty();
      }
    }

    // mutex provides low level access to the mutex, used for shared locking.
    std::mutex& mutex() { return _mutex; }

   private:
    std::mutex _mutex;
    std::deque<std::shared_ptr<RequestItem>> _queue;
  };
  SendQueue _sendQueue;

  // MessageStore keeps a thread safe list of all requests that are "in-flight".
  class MessageStore {
   public:
    // add a given item to the store (indexed by its ID).
    void add(std::shared_ptr<RequestItem> item) {
      std::lock_guard<std::mutex> lockMap(_mutex);
      _map.emplace(item->_messageID, item);
    }

    // findByID returns the item with given ID or nullptr is no such ID is
    // found in the store.
    std::shared_ptr<RequestItem> findByID(MessageID id) {
      std::lock_guard<std::mutex> lockMap(_mutex);
      auto found = _map.find(id);
      if (found == _map.end()) {
        // ID not found
        return std::shared_ptr<RequestItem>();
      }
      return found->second;
    }

    // removeByID removes the item with given ID from the store.
    void removeByID(MessageID id) {
      std::lock_guard<std::mutex> lockMap(_mutex);
      _map.erase(id);
    }

    // Notify all items that their being cancelled (by calling their onError)
    // and remove all items from the store.
    void cancelAll() {
      std::lock_guard<std::mutex> lockMap(_mutex);
      for (auto& item : _map) {
        item.second->_onError(errorToInt(ErrorCondition::CanceledDuringReset),
                              std::move(item.second->_request), nullptr);
      }
      _map.clear();
    }

    // size returns the number of elements in the store.
    size_t size() {
      std::lock_guard<std::mutex> lockMap(_mutex);
      return _map.size();
    }

    // empty returns true when there are no elements in the store, false
    // otherwise.
    bool empty(bool unlocked = false) {
      if (unlocked) {
        return _map.empty();
      } else {
        std::lock_guard<std::mutex> lockMap(_mutex);
        return _map.empty();
      }
    }

    // mutex provides low level access to the mutex, used for shared locking.
    std::mutex& mutex() { return _mutex; }

    // keys returns a string representation of all MessageID's in the store.
    std::string keys() {
      std::lock_guard<std::mutex> lockMap(_mutex);
      return mapToKeys(_map);
    }

   private:
    std::mutex _mutex;
    std::map<MessageID, std::shared_ptr<RequestItem>> _map;
  };
  MessageStore _messageStore;

};
}
}}}
#endif
