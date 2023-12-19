#include "param_sfo.h"
#include <stdexcept>
#include <algorithm>

void param_sfo::param_sfo_file::read_header(std::ifstream& in_stream) {
 byte buffer[4];
 //read magic
 in_stream.seekg(0, std::ios::beg); 
 in_stream.read((char*) buffer, 4);
 this->header.magic = to_uint(buffer, 4);
 if(this->header.magic != this->MAGIC) throw std::invalid_argument("Invalid PARAM.SFO file");
 //read version
 in_stream.seekg(4, std::ios::beg);
 in_stream.read((char*) buffer, 4);
 this->header.version = to_uint(buffer, 4);
 //read keys_table_offset
 in_stream.seekg(8, std::ios::beg);
 in_stream.read((char*) buffer, 4);
 this->header.keys_table_offset = to_uint(buffer, 4);
 //read data_table_offset
 in_stream.seekg(12, std::ios::beg);
 in_stream.read((char*) buffer, 4);
 this->header.data_table_offset = to_uint(buffer, 4);
 //read table_entries
 in_stream.seekg(16, std::ios::beg);
 in_stream.read((char*) buffer, 4);
 this->header.tables_entries = to_uint(buffer, 4);
}

void param_sfo::param_sfo_file::read_index_table_entry(std::ifstream& in_stream, index_table_entry& index_table_entry, uint i) {
 byte buffer[4];
 //read key_offset
 in_stream.seekg(20 + 16 * i, std::ios::beg);
 in_stream.read((char*) buffer, 2);
 index_table_entry.key_offset = (ushort) to_uint(buffer, 2);
 //read datum_fmt
 in_stream.seekg(22 + 16 * i, std::ios::beg);
 in_stream.read((char*) buffer, 2);
 index_table_entry.datum_fmt = (ushort) to_uint(buffer, 2);
 //read datum_len
 in_stream.seekg(24 + 16 * i, std::ios::beg);
 in_stream.read((char*) buffer, 4);
 index_table_entry.datum_len = to_uint(buffer, 4);
 //read datum_max_len
 in_stream.seekg(28 + 16 * i, std::ios::beg);
 in_stream.read((char*) buffer, 4);
 index_table_entry.datum_max_len = to_uint(buffer, 4);
 //read datum_offset
 in_stream.seekg(32 + 16 * i, std::ios::beg);
 in_stream.read((char*) buffer, 4);
 index_table_entry.datum_offset = to_uint(buffer, 4);
}

void param_sfo::param_sfo_file::read_index_table(std::ifstream& in_stream) {
 for(uint i = 0; i < this->header.tables_entries; i++) {
  index_table_entry index_table_entry;
  read_index_table_entry(in_stream, index_table_entry, i);
  this->index_table.entries.push_back(index_table_entry);
 }
}

//PARAM.SFO numbers are stored as little-endian byte arrays, so we need to translate them to big-endian integers
uint param_sfo::param_sfo_file::to_uint(byte LE_array[], int size) {
 uint num = 0;
 for(int i = size - 1; i >= 0; i--) num = (num << 8) + LE_array[i];
 return num;
}

//constructor
param_sfo::param_sfo_file::param_sfo_file(std::string& path) {
 this->path = path;
 std::ifstream in_stream(this->path, std::ios::in | std::ios::binary);
 if(!in_stream.is_open()) throw std::invalid_argument("File not found");
 read_header(in_stream);
 read_index_table(in_stream);
 in_stream.close();
}

void param_sfo::param_sfo_file::print(std::ostream& out_stream) {
 std::ifstream in_stream(this->path, std::ios::in | std::ios::binary);
 if(!in_stream.is_open()) throw std::invalid_argument("File not found");
 //iterate over all entries
 for(uint i = 0; i < this->index_table.entries.size(); i++) {
  std::string data;
  index_table_entry index_table_entry = this->index_table.entries[i];
  //read key value of current entry
  in_stream.seekg(index_table_entry.key_offset + this->header.keys_table_offset, std::ios::beg);
  while(in_stream.peek() != 0) data += (byte) in_stream.get();
  data += " - Length: " + std::to_string(index_table_entry.datum_max_len) + " - Type: " + std::to_string(index_table_entry.datum_fmt) + "\n";
  //read datum value of current entry
  in_stream.seekg(index_table_entry.datum_offset + this->header.data_table_offset, std::ios::beg);
  for(uint k = 0; k < index_table_entry.datum_max_len; k++) data += (byte) in_stream.get();
  out_stream << data << std::endl << std::endl;
 }
 in_stream.close();
}