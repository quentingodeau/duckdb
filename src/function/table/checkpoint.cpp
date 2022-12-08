#include "duckdb/function/table/range.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/storage/storage_manager.hpp"
#include "duckdb/transaction/transaction_manager.hpp"
#include "duckdb/main/database_manager.hpp"

namespace duckdb {

static unique_ptr<FunctionData> CheckpointBind(ClientContext &context, TableFunctionBindInput &input,
                                               vector<LogicalType> &return_types, vector<string> &names) {
	return_types.emplace_back(LogicalType::BOOLEAN);
	names.emplace_back("Success");
	return nullptr;
}

template <bool FORCE>
static void TemplatedCheckpointFunction(ClientContext &context, TableFunctionInput &data_p, DataChunk &output) {
	// FIXME: select database?
	auto &db_manager = DatabaseManager::Get(context);
	auto db = db_manager.GetDatabase(context, db_manager.GetDefaultDatabase());
	auto &transaction_manager = TransactionManager::Get(*db);
	transaction_manager.Checkpoint(context, FORCE);
}

void CheckpointFunction::RegisterFunction(BuiltinFunctions &set) {
	TableFunction checkpoint("checkpoint", {}, TemplatedCheckpointFunction<false>, CheckpointBind);
	set.AddFunction(checkpoint);
	TableFunction force_checkpoint("force_checkpoint", {}, TemplatedCheckpointFunction<true>, CheckpointBind);
	set.AddFunction(force_checkpoint);
}

} // namespace duckdb
