/* Copyright 2015 Stanford University, NVIDIA Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __LEGION_PROFILING_H__
#define __LEGION_PROFILING_H__

#include "lowlevel.h"
#include "utilities.h"
#include "legion_types.h"
#include "legion_utilities.h"
#include "realm/profiling.h"

#include <cassert>
#include <deque>
#include <algorithm>

namespace LegionRuntime {
  namespace HighLevel {

    class LegionProfInstance {
    public:
      struct TaskKind {
      public:
        Processor::TaskFuncID task_id;
        const char *task_name;
      };
      struct TaskVariant {
      public:
        Processor::TaskFuncID func_id;
        const char *variant_name;
      };
      struct OperationInstance {
      public:
        UniqueID op_id;
        unsigned op_kind;
      };
      struct MultiTask {
      public:
        UniqueID op_id;
        Processor::TaskFuncID task_id;
      };
      struct TaskInfo {
      public:
        UniqueID task_id;
        Processor::TaskFuncID func_id;
        Processor proc;
        unsigned long long create, ready, start, stop;
      };
      struct MetaInfo {
      public:
        UniqueID op_id;
        unsigned hlr_id;
        Processor proc;
        unsigned long long create, ready, start, stop;
      };
      struct CopyInfo {
      public:
        UniqueID op_id;
        Memory source, target;
        unsigned long long create, ready, start, stop;
      };
      struct FillInfo {
      public:
        UniqueID op_id;
        Memory target;
        unsigned long long create, ready, start, stop;
      };
      struct InstInfo {
      public:
        UniqueID op_id; 
        PhysicalInstance inst;
        Memory mem;
        size_t total_bytes;
        unsigned long long create, destroy;
      };
    public:
      LegionProfInstance(LegionProfiler *owner);
      LegionProfInstance(const LegionProfInstance &rhs);
      ~LegionProfInstance(void);
    public:
      LegionProfInstance& operator=(const LegionProfInstance &rhs);
    public:
      void register_task_kind(Processor::TaskFuncID kind, const char *name);
      void register_task_variant(const char *variant_name,
                                 const TaskVariantCollection::Variant &variant);
      void register_operation(Operation *op);
      void register_multi_task(Operation *op, Processor::TaskFuncID kind);
    public:
      void process_task(size_t id, UniqueID op_id, 
                  Realm::ProfilingMeasurements::OperationTimeline *timeline,
                  Realm::ProfilingMeasurements::OperationProcessorUsage *usage);
      void process_meta(size_t id, UniqueID op_id,
                  Realm::ProfilingMeasurements::OperationTimeline *timeline,
                  Realm::ProfilingMeasurements::OperationProcessorUsage *usage);
      void process_copy(UniqueID op_id,
                  Realm::ProfilingMeasurements::OperationTimeline *timeline,
                  Realm::ProfilingMeasurements::OperationMemoryUsage *usage);
      void process_fill(UniqueID op_id,
                  Realm::ProfilingMeasurements::OperationTimeline *timeline,
                  Realm::ProfilingMeasurements::OperationMemoryUsage *usage);
      void process_inst(UniqueID op_id,
                  Realm::ProfilingMeasurements::InstanceTimeline *timeline,
                  Realm::ProfilingMeasurements::InstanceMemoryUsage *usage);
    public:
      void dump_state(void);
    private:
      LegionProfiler *const owner;
      std::deque<TaskKind>          task_kinds;
      std::deque<TaskVariant>       task_variants;
      std::deque<OperationInstance> operation_instances;
      std::deque<MultiTask>         multi_tasks;
    private:
      std::deque<TaskInfo> task_infos;
      std::deque<MetaInfo> meta_infos;
      std::deque<CopyInfo> copy_infos;
      std::deque<FillInfo> fill_infos;
      std::deque<InstInfo> inst_infos;
    };

    class LegionProfiler {
    public:
      enum ProfilingKind {
        LEGION_PROF_TASK,
        LEGION_PROF_META,
        LEGION_PROF_COPY,
        LEGION_PROF_FILL,
        LEGION_PROF_INST,
      };
      struct ProfilingInfo {
      public:
        ProfilingInfo(ProfilingKind k)
          : kind(k) { }
      public:
        ProfilingKind kind;
        size_t id;
        UniqueID op_id;
      };
    public:
      // Statically known information passed through the constructor
      // so that it can be deduplicated
      LegionProfiler(Processor target_proc, const Machine &machine,
                     unsigned num_meta_tasks,
                     const char *const *const meta_task_descriptions,
                     unsigned num_operation_kinds,
                     const char *const *const operation_kind_descriptions);
      LegionProfiler(const LegionProfiler &rhs);
      ~LegionProfiler(void);
    public:
      LegionProfiler& operator=(const LegionProfiler &rhs);
    public:
      // Dynamically created things must be registered at runtime
      // Tasks
      void register_task_kind(Processor::TaskFuncID task_id,
                              const char *task_name);
      void register_task_variant(const char *variant_name,
                                 const TaskVariantCollection::Variant &variant);
      // Operations
      void register_operation(Operation *op);
      void register_multi_task(Operation *op, Processor::TaskFuncID task_id);
    public:
      void add_task_request(Realm::ProfilingRequestSet &requests, 
                            Processor::TaskFuncID tid, SingleTask *task);
      void add_meta_request(Realm::ProfilingRequestSet &requests,
                            HLRTaskID tid, Operation *op);
      void add_copy_request(Realm::ProfilingRequestSet &requests, 
                            Operation *op);
      void add_fill_request(Realm::ProfilingRequestSet &requests,
                            Operation *op);
      void add_inst_request(Realm::ProfilingRequestSet &requests,
                            Operation *op);
    public:
      // Process low-level runtime profiling results
      void process_results(Processor p, const void *buffer, size_t size);
    public:
      // Dump all the results
      void finalize(void);
    public:
      const Processor target_proc;
    private:
      LegionProfInstance **const instances;
    };
  };
};

#endif // __LEGION_PROFILING_H__

