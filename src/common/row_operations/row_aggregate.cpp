//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/common/types/row_operations/row_aggregate.cpp
//
//
//===----------------------------------------------------------------------===//
#include "duckdb/common/row_operations/row_operations.hpp"

#include "duckdb/common/types/row_layout.hpp"
#include "duckdb/catalog/catalog_entry/aggregate_function_catalog_entry.hpp"
#include "duckdb/execution/expression_executor.hpp"

namespace duckdb {

void RowOperations::InitializeStates(RowLayout &layout, Vector &addresses, const SelectionVector &sel, idx_t count) {
	if (count == 0) {
		return;
	}
	auto pointers = FlatVector::GetData<data_ptr_t>(addresses);
	auto &offsets = layout.GetOffsets();
	auto aggr_idx = layout.ColumnCount();

	for (auto &aggr : layout.GetAggregates()) {
		for (idx_t i = 0; i < count; ++i) {
			auto row_idx = sel.get_index(i);
			auto row = pointers[row_idx];
			aggr.function.initialize(row + offsets[aggr_idx]);
		}
		++aggr_idx;
	}
}

void RowOperations::DestroyStates(RowLayout &layout, Vector &addresses, idx_t count) {
	if (count == 0) {
		return;
	}
	//	Move to the first aggregate state
	VectorOperations::AddInPlace(addresses, layout.GetAggrOffset(), count);
	for (auto &aggr : layout.GetAggregates()) {
		if (aggr.function.destructor) {
			aggr.function.destructor(addresses, count);
		}
		// move to the next aggregate state
		VectorOperations::AddInPlace(addresses, aggr.payload_size, count);
	}
}

void RowOperations::UpdateStates(AggregateObject &aggr, Vector &addresses, DataChunk &payload, idx_t arg_idx,
                                 idx_t count) {
	aggr.function.update(aggr.child_count == 0 ? nullptr : &payload.data[arg_idx], aggr.bind_data, aggr.child_count,
	                     addresses, count);
}

void RowOperations::UpdateFilteredStates(AggregateObject &aggr, Vector &addresses, DataChunk &payload, idx_t arg_idx) {
	ExpressionExecutor filter_execution(aggr.filter);
	SelectionVector true_sel(STANDARD_VECTOR_SIZE);
	auto count = filter_execution.SelectExpression(payload, true_sel);

	DataChunk filtered_payload;
	auto pay_types = payload.GetTypes();
	filtered_payload.Initialize(pay_types);
	filtered_payload.Slice(payload, true_sel, count);

	Vector filtered_addresses;
	filtered_addresses.Slice(addresses, true_sel, count);
	filtered_addresses.Normalify(count);

	UpdateStates(aggr, filtered_addresses, filtered_payload, arg_idx, filtered_payload.size());
}

} // namespace duckdb
