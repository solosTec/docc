/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_SCRIPT_TASK_RULER_H
#define DOCC_SCRIPT_TASK_RULER_H

#include <context.h>

#include <cyng/task/task_fwd.h>
#include <cyng/task/channel.h>
#include <cyng/task/controller.h>
#include <cyng/task/stash.h>

namespace docscript {

    class ruler {
        template <typename T> friend class cyng::task;

        using signatures_t = std::tuple<
            std::function<void(std::filesystem::path)>,
            std::function<void(std::filesystem::path, std::size_t)>,
            std::function<void(std::filesystem::path, std::size_t)>,
            std::function<void(cyng::eod)>
        >;

    public:
        ruler(
            cyng::channel_weak,
            cyng::controller&,
            cyng::stash&,
            context&);

    private:
        void stop(cyng::eod);
        void read(std::filesystem::path);
        void open(std::filesystem::path, std::size_t);
        void close(std::filesystem::path, std::size_t);
    private:
        signatures_t sigs_;
        cyng::controller& ctl_;
        cyng::stash& stash_;
        cyng::channel_weak channel_;
        context& ctx_;
        std::size_t counter_;   //!< running reader
    };
}

#endif  //  DOCC_SCRIPT_TASK_RULER_H