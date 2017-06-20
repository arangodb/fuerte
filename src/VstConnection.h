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
/// @author Ewout Prangsma
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

#include <fuerte/connection.h>
#include <fuerte/helper.h>
#include <fuerte/loop.h>

#include "vst.h"
#include "MessageStore.h"

// naming in this file will be closer to asio for internal functions and types
// functions that are exposed to other classes follow ArangoDB conding conventions

namespace arangodb { namespace fuerte { inline namespace v1 {

class Loop;

namespace vst {

class VstConnection : public Connection {

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
  virtual ~VstConnection();

public:
  // this function prepares the request for sending
  // by creating a RequestItem and setting:
  //  - a messageid
  //  - the buffer to be send
  // this item is then moved to the request queue
  // and a write action is triggerd when there is
  // no other write in progress
  MessageID sendRequest(std::unique_ptr<Request>, RequestCallback) override;

 private: 
  // Activate the connection.
  virtual void start() override;
  // Return the number of unfinished requests.
  virtual size_t requestsLeft() override;

private:
  // SOCKET HANDLING /////////////////////////////////////////////////////////
  void initSocket();
  void shutdownSocket();
  void shutdownConnection(const ErrorCondition = ErrorCondition::CanceledDuringReset);
  void restartConnection(const ErrorCondition = ErrorCondition::CanceledDuringReset);

  // resolve the host into a series of endpoints 
  void startResolveHost();

  // establishes connection and initiates handshake
  void startConnect(boost::asio::ip::tcp::resolver::iterator);
  void asyncConnectCallback(boost::system::error_code const& ec, boost::asio::ip::tcp::resolver::iterator);

  // start intiating an SSL connection (on top of an established TCP socket)
  void startSSLHandshake();

  // socket connection is up (with optional SSL), now initiate the VST protocol.
  void finishInitialization();
  // Insert all requests needed for authenticating a new connection at the front of the send queue.
  void insertAuthenticationRequests();

  // createRequestItem prepares a RequestItem for the given parameters.
  std::shared_ptr<RequestItem> createRequestItem(std::unique_ptr<Request> request, RequestCallback cb);

  class ReadLoop;

  // activate the receiver loop (if needed)
  void startReading();
  // release the ReadLoop so it will terminate.
  void stopReading();
  // called by a ReadLoop to decide if it must stop.
  // returns true when the given loop should stop.
  bool shouldStopReading(const ReadLoop*, std::chrono::milliseconds& timeout);
  // Restart the connection if the given ReadLoop is still the current read loop.
  void restartConnection(const ReadLoop*, const ErrorCondition);

  // Process the given incoming chunk.
  void processChunk(ChunkHeader &chunk);
  // Create a response object for given RequestItem & received response buffer.
  std::unique_ptr<Response> createResponse(RequestItem& item, std::unique_ptr<VBuffer>& responseBuffer);

  class WriteLoop;

  // activate the sending loop (if needed)
  void startWriting();
  // release the WriteLoop so it will terminate.
  void stopWriting();
  // called by a WriteLoop to request for the next request that will be written.
  // If there is no more work, nullptr is returned and the given loop must stop.
  std::shared_ptr<RequestItem> getNextRequestToSend(const WriteLoop*);
  // Restart the connection if the given WriteLoop is still the current read loop.
  void restartConnection(const WriteLoop*, const ErrorCondition);

private:
  const VSTVersion _vstVersion;
  // TODO FIXME -- fix alignment when done so mutexes are not on the same cacheline etc
  std::atomic_uint_least64_t _messageID;
  // host resolving 
  std::shared_ptr<boost::asio::ip::tcp::resolver> _resolver;
  // socket
  const std::shared_ptr<::boost::asio::io_service> _ioService;
  std::mutex _socket_mutex;
  std::shared_ptr<::boost::asio::ip::tcp::socket> _socket;
  boost::asio::ssl::context _context;
  std::shared_ptr<::boost::asio::ssl::stream<::boost::asio::ip::tcp::socket&>> _sslSocket;
  boost::asio::ip::tcp::resolver::iterator _endpoints;
  boost::asio::ip::tcp::endpoint _peer;
  std::atomic<bool> _permanent_failure;
  std::atomic<uint64_t> _async_calls;
  // reset
  std::atomic_bool _connected;
  struct {
    std::mutex _mutex;
    std::shared_ptr<ReadLoop> _current;
  } _readLoop;
  struct {
    std::mutex _mutex;
    std::shared_ptr<WriteLoop> _current;
  } _writeLoop;
  //queues

  // SendQueue encapsulates a thread safe queue containing RequestItem's that 
  // need sending to the server.
  class SendQueue {
   public:
     // add the given item to the end of the queue.
    void add(std::shared_ptr<RequestItem>& item) {
      std::lock_guard<std::mutex> lockQueue(_mutex);
      _queue.push_back(item);
    }

     // insert the given item to the front of the queue.
    void insert(std::shared_ptr<RequestItem>& item) {
      std::lock_guard<std::mutex> lockQueue(_mutex);
      _queue.insert(_queue.begin(), item);
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
  
  MessageStore<RequestItem> _messageStore;

  // Encapsulate a single read loop on a given socket for a given connection.
  class ReadLoop : public std::enable_shared_from_this<ReadLoop> {
   public:
    ReadLoop(const std::shared_ptr<VstConnection>& connection, const std::shared_ptr<::boost::asio::ip::tcp::socket>& socket) 
      : _connection(connection), _socket(socket), _started(false), _deadline(*(connection->_ioService)) {}
    ~ReadLoop() {
      _deadline.cancel();
    }

    // Start the read loop.
    void start();

   private:
    // reads data from socket with boost::asio::async_read
    void readNextBytes();
    // handler for boost::asio::async_read that extracs chunks form the network
    // takes complete chunks form the socket and starts a new read action. After
    // triggering the next read it processes the received data.
    void asyncReadCallback(boost::system::error_code const&, std::size_t transferred);
    // handler for deadline timer
    void deadlineHandler(const boost::system::error_code& error);

   private:
    std::shared_ptr<VstConnection> _connection;
    std::shared_ptr<::boost::asio::ip::tcp::socket> _socket;
    ::boost::asio::streambuf _receiveBuffer; // async read can not run concurrent
    std::atomic_bool _started;
    ::boost::asio::deadline_timer _deadline;
  };

  // Encapsulate a single write loop on a given socket for a given connection.
  class WriteLoop : public std::enable_shared_from_this<WriteLoop> {
   public:
    WriteLoop(const std::shared_ptr<VstConnection>& connection, const std::shared_ptr<::boost::asio::ip::tcp::socket>& socket) 
      : _connection(connection), _socket(socket), _started(false), _deadline(*(connection->_ioService)) {}
    ~WriteLoop() {
      _deadline.cancel();
    }

    // Start the write loop.
    void start();

   private:
    // writes data from task queue to network using boost::asio::async_write
    void sendNextRequest();
    // handler for boost::asio::async_wirte that calls startWrite as long as there is new data
    void asyncWriteCallback(boost::system::error_code const&, std::size_t transferred, std::shared_ptr<RequestItem>);
    // handler for deadline timer
    void deadlineHandler(const boost::system::error_code& error);

   private:
    std::shared_ptr<VstConnection> _connection;
    std::shared_ptr<::boost::asio::ip::tcp::socket> _socket;
    std::atomic_bool _started;
    ::boost::asio::deadline_timer _deadline;
  };

};
}
}}}
#endif
