#include "param_sfo.h"
#include <stdexcept>
#include <algorithm>
#include <iomanip>

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

void param_sfo::param_sfo_file::read_index_table_entry(std::ifstream& in_stream, uint i, index_table_entry_t& index_table_entry) {
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
  index_table_entry_t index_table_entry;
  this->read_index_table_entry(in_stream, i, index_table_entry);
  this->index_table.entries.push_back(index_table_entry);
 }
}

void param_sfo::param_sfo_file::read_param_entry(std::ifstream& in_stream, index_table_entry_t& index_table_entry, param_entry_t& param_entry) {
 //read key of current entry
 in_stream.seekg(index_table_entry.key_offset + this->header.keys_table_offset, std::ios::beg);
 while(in_stream.peek() != '\0') param_entry.keys_table_entry += in_stream.get();
 param_entry.key_len = param_entry.keys_table_entry.size();
 //read datum of current entry
 in_stream.seekg(index_table_entry.datum_offset + this->header.data_table_offset, std::ios::beg);
 switch(index_table_entry.datum_fmt) {
  case fmt::utf8:
   param_entry.datum_fmt = "utf8";
   param_entry.datum_len = index_table_entry.datum_len;
   break;
  case fmt::utf8null:
   param_entry.datum_fmt = "utf8null";
   param_entry.datum_len = index_table_entry.datum_len;
   break;
  case fmt::uint32:
   param_entry.datum_fmt = "uint32";
   param_entry.datum_len = 4;
   break;
  default:
   param_entry.datum_fmt = "";
   param_entry.datum_len = 0;
 }
 for(uint i = 0; i < param_entry.datum_len; i++) param_entry.data_table_entry += in_stream.get();
 param_entry.data_table_entry.erase(remove(param_entry.data_table_entry.begin(), param_entry.data_table_entry.end(), '\0'), param_entry.data_table_entry.end());
}

void param_sfo::param_sfo_file::read_param_table(std::ifstream& in_stream) {
 for(index_table_entry_t index_table_entry : this->index_table.entries) {
  param_entry_t param_entry;
  this->read_param_entry(in_stream, index_table_entry, param_entry);
  this->param_table.entries.push_back(param_entry);
 }
}

//PARAM.SFO numbers are stored as little-endian byte arrays, so it is necessary to translate them to big-endian
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
 this->read_header(in_stream);
 this->read_index_table(in_stream);
 this->read_param_table(in_stream);
 in_stream.close();
}

void param_sfo::param_sfo_file::print(std::ostream& out_stream, bool hex_format) {
 out_stream << "PATH" << std::endl << this->path << std::endl << std::endl;
 out_stream << "SFO VERSION" << std::endl;
 if(!hex_format) out_stream << this->header.version << std::endl << std::endl;
 else out_stream << std::hex << std::setfill('0') << std::setw(4) << std::uppercase << this->header.version << std::endl << std::endl;
 for(param_entry_t param_entry : this->param_table.entries) {
  out_stream << param_entry.keys_table_entry + " - Fmt: " + param_entry.datum_fmt << std::endl;
  if(!hex_format) out_stream << param_entry.data_table_entry << std::endl << std::endl;
  else {
   for(size_t i = 0; i < param_entry.data_table_entry.size(); i++) out_stream << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << static_cast<uint>(param_entry.data_table_entry[i]);
   out_stream << std::endl << std::endl;
  }
 }
}