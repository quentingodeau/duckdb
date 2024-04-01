//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/serializer/buffered_file_writer.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/serializer/write_stream.hpp"
#include "duckdb/common/file_system.hpp"
#include "duckdb/common/typedefs.hpp"

namespace duckdb {

#define FILE_BUFFER_SIZE 4096

class BufferedFileWriter : public WriteStream {
public:
	static constexpr uint8_t DEFAULT_OPEN_FLAGS = FileFlags::FILE_FLAGS_WRITE | FileFlags::FILE_FLAGS_FILE_CREATE;
	static constexpr idx_t DEFAULT_BUFFER_SIZE = 4096;

	//! Serializes to a buffer allocated by the serializer
	DUCKDB_API BufferedFileWriter(FileSystem &fs, const string &path, uint8_t open_flags = DEFAULT_OPEN_FLAGS, idx_t buffer_size = DEFAULT_BUFFER_SIZE);

	FileSystem &fs;
	const string path;
	const idx_t buffer_size;
	unsafe_unique_array<data_t> data;
	idx_t offset;
	idx_t total_written;
	unique_ptr<FileHandle> handle;

public:
	DUCKDB_API void WriteData(const_data_ptr_t buffer, idx_t write_size) override;
	//! Flush the buffer to disk and sync the file to ensure writing is completed
	DUCKDB_API void Sync();
	//! Flush the buffer to the file (without sync)
	DUCKDB_API void Flush();
	//! Returns the current size of the file
	DUCKDB_API int64_t GetFileSize();
	//! Truncate the size to a previous size (given that size <= GetFileSize())
	DUCKDB_API void Truncate(int64_t size);

	DUCKDB_API idx_t GetTotalWritten();
};

} // namespace duckdb
