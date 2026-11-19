// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once
#include <cstddef>
#include <cstdint>

// Export visibility
#if defined(_WIN32)
#ifdef EXAMPLE_PLUGIN_EP_BUILD
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL __declspec(dllimport)
#endif
#elif defined(__APPLE__)
#define EXPORT_SYMBOL __attribute__((visibility("default")))
#else
#define EXPORT_SYMBOL
#endif

inline constexpr const char* kExampleEpTestEpContextDataSupport =
    "ep.example_ep.test_ep_context_data_support";
inline constexpr const char* kExampleEpTestOrtVersion = "ep.example_ep.test_ort_version";
// Overrides the OrtWeightlessSupport value returned by GetWeightlessSupport() (base-10 integer), to make it disagree
// with the EP metadata.
inline constexpr const char* kExampleEpTestWeightlessSupport = "ep.example_ep.test_weightless_support";
// "1" makes the EP obtain the initializer data itself in weightless mode (drop_constant_initializers = true), keeping
// references to the initializers it doesn't copy, instead of letting ORT provide the initializers. Only for compiling:
// the EP doesn't actually load the referenced data.
inline constexpr const char* kExampleEpTestWeightlessDropInitializers =
    "ep.example_ep.test_weightless_drop_initializers";
// Declared location of a source model external data file (e.g., "weights/model.data"). When set, the EP reads that file
// while it imports an EPContext node, as an EP that rebuilds the source model of a weightless EPContext model does: via
// the EPContext read callback, or, if the callback doesn't supply it, from the source model's directory
// ("session.model_external_initializers_file_folder_path", or the directory of "ep.context_source_model_path"). The EP
// reads the file once per session and keeps it until it's released.
inline constexpr const char* kExampleEpTestSourceModelDataFile = "ep.example_ep.test_source_model_data_file";

extern "C" {
EXPORT_SYMBOL void ExampleEpTestHooks_ResetSyncCount();
EXPORT_SYMBOL uint64_t ExampleEpTestHooks_GetSyncCount();
EXPORT_SYMBOL void ExampleEpTestHooks_ResetPreallocatedOutputQuery();
EXPORT_SYMBOL int ExampleEpTestHooks_GetPreallocatedOutputQueryResult();
EXPORT_SYMBOL int ExampleEpTestHooks_GetPreallocatedOutputBadIndexRejected();
EXPORT_SYMBOL void ExampleEpTestHooks_SetCreateDataTransferFailure(int enabled);
// Value of the deprecated "ep.enable_weightless" session option seen by the last CreateEp() call:
// -2: CreateEp() not called since the last reset, -1: not set, 0: "0", 1: "1", 2: any other value.
// ExampleEpTestHooks_ResetEnableWeightlessOption() also resets the value returned by
// ExampleEpTestHooks_GetEnableWeightlessEpContextNodesOption(), which uses the same encoding for the deprecated
// "ep.enable_weightless_ep_context_nodes" option.
EXPORT_SYMBOL void ExampleEpTestHooks_ResetEnableWeightlessOption();
EXPORT_SYMBOL int ExampleEpTestHooks_GetEnableWeightlessOption();
EXPORT_SYMBOL int ExampleEpTestHooks_GetEnableWeightlessEpContextNodesOption();
// Number of constant initializers the EP copied since the last reset.
EXPORT_SYMBOL void ExampleEpTestHooks_ResetSavedInitializerCount();
EXPORT_SYMBOL uint64_t ExampleEpTestHooks_GetSavedInitializerCount();
// Weightless source model buffer seen by the last CreateEp() call (nullptr and 0 if not set or after a reset).
EXPORT_SYMBOL void ExampleEpTestHooks_ResetWeightlessSourceModelBuffer();
EXPORT_SYMBOL void ExampleEpTestHooks_GetWeightlessSourceModelBuffer(const void** data, size_t* length);
// Value the factory reports in the older "weightless_support" EP metadata entry for EP devices created afterwards,
// i.e., when the library is registered. nullptr restores the default value ("all").
EXPORT_SYMBOL void ExampleEpTestHooks_SetLegacyWeightlessSupportMetadata(const char* value);
// Source model data file the EP read for kExampleEpTestSourceModelDataFile since the last reset: the number of reads,
// the address and size of the data the EP used, whether the read callback supplied it (1) or the EP read the file (0),
// and an FNV-1a checksum of the data when the EP read it and when the EP was released (0 if not released yet).
EXPORT_SYMBOL void ExampleEpTestHooks_ResetSourceModelDataFile();
EXPORT_SYMBOL void ExampleEpTestHooks_GetSourceModelDataFile(uint64_t* read_count, const void** data, size_t* size,
                                                             int* supplied_by_application, uint64_t* checksum_at_read,
                                                             uint64_t* checksum_at_release);
}

// Internal to the library; not exported.
void RecordPreallocatedOutputQueryResult(int has_preallocated_output);
void RecordPreallocatedOutputBadIndexRejected(int rejected);
bool ShouldFailCreateDataTransfer();
void RecordEnableWeightlessOption(int value);
void RecordEnableWeightlessEpContextNodesOption(int value);
void RecordSavedInitializer();
void RecordWeightlessSourceModelBuffer(const void* data, size_t length);
uint64_t SourceModelDataChecksum(const void* data, size_t size);
void RecordSourceModelDataFileRead(const void* data, size_t size, bool supplied_by_application);
void RecordSourceModelDataFileRelease(const void* data, size_t size);
const char* GetLegacyWeightlessSupportMetadata();
