#include "reassembler.hh"
#include "debug.hh"

using namespace std;


bool Reassembler::try_push( Writer& my_writer, uint64_t first_index, string data ){
  if ((first_index == next_index_ ) || (first_index < next_index_ && first_index + data.size() > next_index_)) {
    const auto capacity = my_writer.available_capacity();
    if (first_index == next_index_) {
      const auto sliced_data = data.substr(0, std::min(capacity, data.size()));
      my_writer.push(sliced_data);
      next_index_ += sliced_data.size();
      if (capacity < data.size()) {
        pending_[first_index + sliced_data.size()] = data.substr( sliced_data.size() );

      }
      return true;
    }

    else {
      const auto sliced_data = data.substr(next_index_ - first_index, std::min(capacity, data.size() - (next_index_ - first_index)));
      my_writer.push(sliced_data);
      if (capacity < data.size() - (next_index_ - first_index)) {
        pending_[next_index_ + sliced_data.size()] = data.substr((next_index_ - first_index) + sliced_data.size());
        // data.size = 10 siliced_data.size = 7 
      }
      next_index_ += sliced_data.size();
      return true;
    }
  }

  return false;
}
void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  // debug( "unimplemented insert({}, {}, {}) called", first_index, data, is_last_substring );
  if (is_last_substring) {
    last_index_ = first_index + data.size();
  }
  Writer& my_writer = output_.writer();
  const auto push_result = try_push(my_writer, first_index, data);
  
  for (auto it = pending_.begin(); it != pending_.end(); ) {
    if ( try_push( my_writer ,it->first, it->second ) ) {
      it = pending_.erase ( it );
    }
    else {
      ++it;
    }
  }

  if ( !push_result && first_index > next_index_) {
    pending_[first_index] = data;
  }

  if (next_index_ >= last_index_) {
    my_writer.close();
  }

}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  // debug( "unimplemented count_bytes_pending() called" );
  uint64_t stored_data = 0;
  for ( const auto& [index, data] : pending_) {
    stored_data += data.size();
  }
  return {stored_data};
}
