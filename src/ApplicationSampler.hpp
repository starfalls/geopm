/*
 * Copyright (c) 2015, 2016, 2017, 2018, 2019, 2020, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY LOG OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef APPLICATIONSAMPLER_HPP_INCLUDE
#define APPLICATIONSAMPLER_HPP_INCLUDE

#include <memory>
#include <vector>

#include "geopm_time.h"

namespace geopm
{
    class ProfileSampler;
    class ProfileIOSample;
    class EpochRuntimeRegulator;

    class ApplicationSampler
    {
        public:
            /// @brief Enumeration of event types that can be stored
            ///        in a record.
            ///
            /// This enum is stored in the "event" field of an
            /// m_record_s.  For each event type the "signal" field of
            /// a record will represent different data.  The
            /// description of the "signal" field in the m_record_s is
            /// given for each enum value.
            enum m_event_e {
                M_EVENT_REGION_ENTRY,     /// EVENT: The application has entered a region.
                                          /// SIGNAL: The hash of the entered region.
                M_EVENT_REGION_EXIT,      /// EVENT: The application has exited a region.
                                          /// SIGNAL: The hash of the exited region.
                ///
                /// @todo SUPPORT FOR EVENTS BELOW IS FUTURE WORK
                ///
                M_EVENT_EPOCH_COUNT,      /// EVENT: The application has begun a new epoch.
                                          /// SIGNAL: The number new epochs begun (1, 2,...)
                M_EVENT_REGISTER_PROFILE, /// EVENT: The application has started up and all
                                          ///        all processes associated with the
                                          ///        application identify their profile name.
                                          /// SIGNAL: The hash of the profile name unique to
                                          ///         the application.
                M_EVENT_CLAIM_CPU,        /// EVENT: The application has started up.  Each
                                          ///        process will send one "claim" event per
                                          ///        CPU in affinity mask.
                                          /// SIGNAL: Linux logical CPU claimed by process.
                M_EVENT_RELEASE_CPU,      /// EVENT: The application is shutting down.  Each
                                          ///        process will send one "release" event for
                                          ///        every previous "claim" event.
                M_EVENT_NAME_KEY,         /// EVENT: The application is shutting down and has
                                          ///        recorded all region names.
                                          /// SIGNAL: A unique identifier which can be used to
                                          ///         access the vector of all strings hashed
                                          ///         by the application (with get_names()).
                M_EVENT_HINT,             /// EVENT: Application behavior hint has changed.
                                          ///        In the future this data will only be
                                          ///        available as sampled by the per_cpu_hint()
                                          ///        method and will not be reported as an event.
                                          /// SIGNAL: The GEOPM_REGION_HINT enum value for the
                                          ///         process.
                M_EVENT_RANK_PROGRESS,    /// EVENT: The application has updated per process
                                          ///        In the future this data will only be
                                          ///        available as sampled by the
                                          ///        per_cpu_progress() method and will not be
                                          ///        reported as an event.
                                          /// SIGNAL: Proccess progress from 0.0 - 1.0.
                M_NUM_EVENT
            };

            /// @brief Record of an application event.
            struct m_record_s {
                /// @brief Elapsed time since time zero when event was
                ///        recorded.
                double time;
                /// @brief The process identifier where event occured.
                int process;
                /// @brief One of the m_event_e event types.
                int event;
                /// @brief The signal associated with the event type.
                double signal;
            };

            /// @brief Singleton accessor for the application sampler.
            static ApplicationSampler &application_sampler(void);
            /// @brief Set the reference time that will be used for
            ///        all future record time reporting.
            /// @param [in] start_time The reference zero time.
            virtual void time_zero(const geopm_time_s &start_time) = 0;
            /// @brief Update the record buffer by clearing out old
            ///        records and providing a new cache for
            ///        subsequent calls to the get_records() method.
            virtual void update_records(void) = 0;
            /// @brief Get all of the application events that have
            ///        been recorded since the last call to
            ///        update_records().
            /// @return Vector of application event records.
            virtual std::vector<m_record_s> get_records(void) const = 0;
            /// @brief Called after recording a M_EVENT_NAME_KEY event
            ///        to get all string hashed by the process.
            ///
            ///        The result includes the names of all entered
            ///        regions and the profile name.  The user can
            ///        then create a reverse mapping of hashes
            ///        recieved in prior events to their string name
            ///        for reporting purposes.
            ///
            /// @param [in] name_key The signal from the name key
            ///        event record.
            /// @return Vector of all strings hashed by the process
            ///         where the hash was returned as part of an
            ///         event.
            virtual std::vector<std::string> get_names(uint64_t name_key) const = 0;
            /// @brief Sample the current hint for every cpu.
            /// @return Vector over Linux logical CPU of the GEOPM
            ///         region hint currently being executed.
            virtual std::vector<uint64_t> per_cpu_hint(void) const = 0;
            /// @brief Sample the current progress for every cpu.
            /// @return Vector over Linux logical CPU of the region
            ///         progress.
            virtual std::vector<double> per_cpu_progress(void) const = 0;

            // Deprecated API's below for access to legacy objects
            virtual void set_sampler(std::shared_ptr<ProfileSampler> sampler) = 0;
            virtual std::shared_ptr<ProfileSampler> get_sampler(void) = 0;
            virtual void set_regulator(std::shared_ptr<EpochRuntimeRegulator> regulator) = 0;
            virtual std::shared_ptr<EpochRuntimeRegulator> get_regulator(void) = 0;
            virtual void set_io_sample(std::shared_ptr<ProfileIOSample> io_sample) = 0;
            virtual std::shared_ptr<ProfileIOSample> get_io_sample(void) = 0;
        protected:
            virtual ~ApplicationSampler() = default;
            ApplicationSampler() = default;
    };
}

#endif
