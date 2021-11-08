/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_ASM_READER_H
#define DOCC_ASM_READER_H

#include <context.h>
#include <sanitizer.h>
#include <tokenizer.h>


namespace docasm {

    class reader {
    public:
        reader(std::filesystem::path out
            , std::vector<std::filesystem::path> const& inc
            , int verbose);
        void read(std::filesystem::path);

    private:
        void next_symbol(symbol&& sym);
    private:
        context ctx_;
        token prev_;
        tokenizer tokenizer_;
        sanitizer sanitizer_;
    };
}

#endif  //  DOCC_SCRIPT_TASK_RULER_H