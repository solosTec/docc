/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_SCRIPT_TASK_READER_H
#define DOCC_SCRIPT_TASK_READER_H

#include <context.h>
#include <sanitizer.h>
#include <tokenizer.h>

#include <cyng/task/task_fwd.h>
#include <cyng/task/channel.h>
#include <cyng/task/controller.h>
#include <cyng/task/stash.h>

namespace docscript {

    class reader {
        template <typename T> friend class cyng::task;

        using signatures_t = std::tuple<
            std::function<void(std::filesystem::path)>,
            std::function<void(cyng::eod)>
        >;

    public:
        reader(
            cyng::channel_weak,
            cyng::controller&,
            cyng::stash&,
            context&);

    private:
        void stop(cyng::eod);
        void read(std::filesystem::path);
        void next_symbol(symbol&& sym);
    private:
        signatures_t sigs_;
        cyng::channel_weak channel_;
        cyng::controller& ctl_;
        cyng::stash& stash_;
        context& ctx_;
        token prev_;
        tokenizer tokenizer_;
        sanitizer sanitizer_;
    };
}

#endif  //  DOCC_SCRIPT_TASK_RULER_H