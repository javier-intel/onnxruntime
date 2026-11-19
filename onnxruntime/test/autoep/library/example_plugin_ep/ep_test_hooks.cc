// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#define EXAMPLE_PLUGIN_EP_BUILD
#include "ep_test_hooks.h"
#include <atomic>
#include <mutex>
#include <string>

std::atomic<uint64_t> g_sync_count{0};
std::atomic<int> g_preallocated_output_query_result{-1};
std::atomic<int> g_preallocated_output_bad_index_rejected{-1};
std::atomic<bool> g_create_data_transfer_should_fail{false};
std::atomic<int> g_enable_weightless_option{-2};
std::atomic<int> g_enable_weightless_ep_context_nodes_option{-2};
std::atomic<uint64_t> g_saved_initializer_count{0};
std::atomic<const void*> g_weightless_source_model_data{nullptr};
std::atomic<size_t> g_weightless_source_model_data_length{0};

extern "C" void ExampleEpTestHooks_ResetSyncCount() { g_sync_count.store(0); }
extern "C" uint64_t ExampleEpTestHooks_GetSyncCount() { return g_sync_count.load(); }
extern "C" void ExampleEpTestHooks_ResetPreallocatedOutputQuery() {
  g_preallocated_output_query_result.store(-1);
  g_preallocated_output_bad_index_rejected.store(-1);
}
extern "C" int ExampleEpTestHooks_GetPreallocatedOutputQueryResult() {
  return g_preallocated_output_query_result.load();
}
extern "C" int ExampleEpTestHooks_GetPreallocatedOutputBadIndexRejected() {
  return g_preallocated_output_bad_index_rejected.load();
}
extern "C" void ExampleEpTestHooks_SetCreateDataTransferFailure(int enabled) {
  g_create_data_transfer_should_fail.store(enabled != 0);
}
void RecordPreallocatedOutputQueryResult(int has_preallocated_output) {
  g_preallocated_output_query_result.store(has_preallocated_output);
}
void RecordPreallocatedOutputBadIndexRejected(int rejected) {
  g_preallocated_output_bad_index_rejected.store(rejected);
}
bool ShouldFailCreateDataTransfer() {
  return g_create_data_transfer_should_fail.load();
}
extern "C" void ExampleEpTestHooks_ResetEnableWeightlessOption() {
  g_enable_weightless_option.store(-2);
  g_enable_weightless_ep_context_nodes_option.store(-2);
}
extern "C" int ExampleEpTestHooks_GetEnableWeightlessOption() { return g_enable_weightless_option.load(); }
extern "C" int ExampleEpTestHooks_GetEnableWeightlessEpContextNodesOption() {
  return g_enable_weightless_ep_context_nodes_option.load();
}
void RecordEnableWeightlessOption(int value) { g_enable_weightless_option.store(value); }
void RecordEnableWeightlessEpContextNodesOption(int value) { g_enable_weightless_ep_context_nodes_option.store(value); }
extern "C" void ExampleEpTestHooks_ResetSavedInitializerCount() { g_saved_initializer_count.store(0); }
extern "C" uint64_t ExampleEpTestHooks_GetSavedInitializerCount() { return g_saved_initializer_count.load(); }
void RecordSavedInitializer() { ++g_saved_initializer_count; }
extern "C" void ExampleEpTestHooks_ResetWeightlessSourceModelBuffer() {
  RecordWeightlessSourceModelBuffer(nullptr, 0);
}
extern "C" void ExampleEpTestHooks_GetWeightlessSourceModelBuffer(const void** data, size_t* length) {
  *data = g_weightless_source_model_data.load();
  *length = g_weightless_source_model_data_length.load();
}
namespace {
std::mutex g_legacy_weightless_support_metadata_mutex;
std::string g_legacy_weightless_support_metadata = "all";
}  // namespace

extern "C" void ExampleEpTestHooks_SetLegacyWeightlessSupportMetadata(const char* value) {
  std::lock_guard<std::mutex> lock(g_legacy_weightless_support_metadata_mutex);
  g_legacy_weightless_support_metadata = value != nullptr ? value : "all";
}
const char* GetLegacyWeightlessSupportMetadata() {
  std::lock_guard<std::mutex> lock(g_legacy_weightless_support_metadata_mutex);
  return g_legacy_weightless_support_metadata.c_str();
}
void RecordWeightlessSourceModelBuffer(const void* data, size_t length) {
  g_weightless_source_model_data.store(data);
  g_weightless_source_model_data_length.store(length);
}

namespace {
struct SourceModelDataFileRecord {
  uint64_t read_count = 0;
  const void* data = nullptr;
  size_t size = 0;
  int supplied_by_application = -1;
  uint64_t checksum_at_read = 0;
  uint64_t checksum_at_release = 0;
};
std::mutex g_source_model_data_file_mutex;
SourceModelDataFileRecord g_source_model_data_file;
}  // namespace

uint64_t SourceModelDataChecksum(const void* data, size_t size) {
  uint64_t hash = 14695981039346656037ULL;  // FNV-1a
  const auto* bytes = static_cast<const unsigned char*>(data);
  for (size_t i = 0; i < size; ++i) {
    hash = (hash ^ bytes[i]) * 1099511628211ULL;
  }
  return hash;
}

void RecordSourceModelDataFileRead(const void* data, size_t size, bool supplied_by_application) {
  const uint64_t checksum = SourceModelDataChecksum(data, size);
  std::lock_guard<std::mutex> lock(g_source_model_data_file_mutex);
  ++g_source_model_data_file.read_count;
  g_source_model_data_file.data = data;
  g_source_model_data_file.size = size;
  g_source_model_data_file.supplied_by_application = supplied_by_application ? 1 : 0;
  g_source_model_data_file.checksum_at_read = checksum;
}

void RecordSourceModelDataFileRelease(const void* data, size_t size) {
  const uint64_t checksum = SourceModelDataChecksum(data, size);
  std::lock_guard<std::mutex> lock(g_source_model_data_file_mutex);
  g_source_model_data_file.checksum_at_release = checksum;
}

extern "C" void ExampleEpTestHooks_ResetSourceModelDataFile() {
  std::lock_guard<std::mutex> lock(g_source_model_data_file_mutex);
  g_source_model_data_file = SourceModelDataFileRecord{};
}

extern "C" void ExampleEpTestHooks_GetSourceModelDataFile(uint64_t* read_count, const void** data, size_t* size,
                                                         int* supplied_by_application, uint64_t* checksum_at_read,
                                                         uint64_t* checksum_at_release) {
  std::lock_guard<std::mutex> lock(g_source_model_data_file_mutex);
  *read_count = g_source_model_data_file.read_count;
  *data = g_source_model_data_file.data;
  *size = g_source_model_data_file.size;
  *supplied_by_application = g_source_model_data_file.supplied_by_application;
  *checksum_at_read = g_source_model_data_file.checksum_at_read;
  *checksum_at_release = g_source_model_data_file.checksum_at_release;
}
