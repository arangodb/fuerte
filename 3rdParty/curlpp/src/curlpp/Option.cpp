#include "curlpp/Option.hpp"
#include "curlpp/internal/OptionSetter.hpp"

template <typename OptionType, CURLoption option>
const CURLoption curlpp::OptionTrait<OptionType, option>::option;
