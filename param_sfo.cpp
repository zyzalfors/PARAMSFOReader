#include "param_sfo.h"

void param_sfo::param_sfo_file::read_header(std::ifstream& stream) {
 byte buffer[4];
 //read magic
 stream.seekg(0, std::ios::beg); 
 stream.read((char*) &buffer[0], 4);
 head.magic = convert_little_endian_array_to_uint(buffer, 4);
 if(head.magic != MAGIC) {
  throw std::invalid_argument("Invalid sfo file");
 }
 //read version
 stream.seekg(4, std::ios::beg);
 stream.read((char*) &buffer[0], 4);
 head.version = convert_little_endian_array_to_uint(buffer, 4);
 //read keys_table_offset
 stream.seekg(8, std::ios::beg);
 stream.read((char*) &buffer[0], 4);
 head.keys_table_offset = convert_little_endian_array_to_uint(buffer, 4);
 //read data_table_offset
 stream.seekg(12, std::ios::beg);
 stream.read((char*) &buffer[0], 4);
 head.data_table_offset = convert_little_endian_array_to_uint(buffer, 4);
 //read table_entries
 stream.seekg(16, std::ios::beg);
 stream.read((char*) &buffer[0], 4);
 head.tables_entries = convert_little_endian_array_to_uint(buffer, 4);
}

void param_sfo::param_sfo_file::read_index_table_entry(std::ifstream& stream, struct index_table_entry& table_entry, uint i) {
 byte buffer[4];
 //read key_offset
 stream.seekg(20 + 16 * i, std::ios::beg);
 stream.read((char*) &buffer[0], 2);
 table_entry.key_offset = (ushort) convert_little_endian_array_to_uint(buffer, 2);
 //read datum_fmt
 stream.seekg(22 + 16 * i, std::ios::beg);
 stream.read((char*) &buffer[0], 2);
 table_entry.datum_fmt = (ushort) convert_little_endian_array_to_uint(buffer, 2);
 //read datum_len
 stream.seekg(24 + 16 * i, std::ios::beg);
 stream.read((char*) &buffer[0], 4);
 table_entry.datum_len = convert_little_endian_array_to_uint(buffer, 4);
 //read datum_max_len
 stream.seekg(28 + 16 * i, std::ios::beg);
 stream.read((char*) &buffer[0], 4);
 table_entry.datum_max_len = convert_little_endian_array_to_uint(buffer, 4);
 //read datum_offset
 stream.seekg(32 + 16 * i, std::ios::beg);
 stream.read((char*) &buffer[0], 4);
 table_entry.datum_offset = convert_little_endian_array_to_uint(buffer, 4);
}

void param_sfo::param_sfo_file::read_index_table(std::ifstream& stream) {
 for(uint i = 0; i < head.tables_entries; i++) {
  struct index_table_entry table_entry;
  read_index_table_entry(stream, table_entry, i);
  table.table_entries.push_back(table_entry);
 }
}

//param_sfo numbers are stored as little-endian byte arrays, so we need to translate them to big-endian integers
uint param_sfo::param_sfo_file::convert_little_endian_array_to_uint(byte array[], int size) {
 uint num = 0;
 for(int i = size - 1; i >= 0; i--) {
  num = (num << 8) + array[i];
 }
 return num;
}

param_sfo::param_sfo_file::param_sfo_file(std::string _filepath) {
 filepath = _filepath;
 std::ifstream stream(filepath, std::ios::in | std::ios::binary);
 if(!stream.is_open()) {
  throw std::invalid_argument("File not found");
 }
 read_header(stream);
 read_index_table(stream);
 stream.close();
}

void param_sfo::param_sfo_file::print(std::ostream& out_stream) {
 std::ifstream stream(filepath, std::ios::in | std::ios::binary);
 for(uint i = 0; i < table.table_entries.size(); i++) {
  std::string key_datum;
  struct index_table_entry table_entry = table.table_entries[i]; 
  if(!stream.is_open()) {
   throw std::invalid_argument("File not found");
  }
  stream.seekg(table_entry.key_offset + head.keys_table_offset, std::ios::beg);
  while(stream.peek() != 0) {
   key_datum += (byte) stream.get();
  }
  key_datum += "\n";
  stream.seekg(table_entry.datum_offset + head.data_table_offset, std::ios::beg);
  switch(table_entry.datum_fmt) {
   case fmt::utf8notnull:
   case fmt::utf8null:
    for(uint k = 0; k < table_entry.datum_max_len; k++) {
     key_datum += (byte) stream.get();
    }
    break;
   case fmt::uint32:
    for(uint k = 0; k < table_entry.datum_max_len; k++) {
     key_datum += std::to_string((byte) stream.get());
    }
    break;
  }
  key_datum.erase(remove(key_datum.begin(), key_datum.end(), '\0'), key_datum.end());
  out_stream<<key_datum<<std::endl<<std::endl;
 }
 stream.close();
}