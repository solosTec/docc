#include <tasks/ruler.h>
#include <tasks/reader.h>

#include <fmt/core.h>
#include <fmt/color.h>

namespace docscript {
    namespace task {

        ruler::ruler(
            cyng::channel_weak wp,
            cyng::controller& ctl,
            cyng::stash& s,
            context& ctx)
            : sigs_{
                std::bind(&ruler::read, this, std::placeholders::_1),
                std::bind(&ruler::open, this, std::placeholders::_1, std::placeholders::_2),
                std::bind(&ruler::close, this, std::placeholders::_1, std::placeholders::_2),
                std::bind(&ruler::stop, this, std::placeholders::_1)
        }
            , ctl_(ctl)
            , stash_(s)
            , channel_(wp)
            , ctx_(ctx)
            , counter_(0)
        {
            auto sp = channel_.lock();
            if (sp) {
                std::size_t slot{ 0 };
                sp->set_channel_name("read", slot++);
                sp->set_channel_name("open", slot++);
                sp->set_channel_name("close", slot++);

                if (ctx_.get_verbosity(1)) {
                    fmt::print(
                        stdout,
                        fg(fmt::color::green_yellow) | fmt::emphasis::bold,
                        "***info : task {} started\n", channel_.lock()->get_name());
                }
            }
        }

        void ruler::stop(cyng::eod) {
            if (ctx_.get_verbosity(1)) {
                fmt::print(
                    stdout,
                    fg(fmt::color::dark_orange) | fmt::emphasis::bold,
                    "***info : task {} stopped\n", channel_.lock()->get_name());
            }
        }

        void ruler::read(std::filesystem::path p) {
            if (ctx_.get_verbosity(1)) {
                fmt::print(
                    stdout,
                    fg(fmt::color::green_yellow) | fmt::emphasis::bold,
                    "***info : read [{}]\n", p.filename().string());
            }

            auto channel = ctl_.create_named_channel_with_ref<reader>("reader", ctl_, stash_, ctx_);
            BOOST_ASSERT(channel->is_open());
            channel->dispatch("read", p);
            stash_.lock(channel);

        }

        void ruler::open(std::filesystem::path p, std::size_t tsk) {
            ++counter_;
            if (ctx_.get_verbosity(2)) {
                fmt::print(
                    stdout,
                    fg(fmt::color::green_yellow) | fmt::emphasis::bold,
                    "***info : #{} opens [{}] \n", tsk, p.string());
            }

        }

        void ruler::close(std::filesystem::path p, std::size_t tsk) {
            --counter_;
            if (ctx_.get_verbosity(2)) {
                fmt::print(
                    stdout,
                    fg(fmt::color::green_yellow) | fmt::emphasis::bold,
                    "***info : #{} closes [{}]\n", tsk, p.string());
            }

            //
            //  stop tasks
            // 
            if (counter_ == 0) {
                if (ctx_.get_verbosity(1)) {
                    fmt::print(
                        stdout,
                        fg(fmt::color::forest_green) | fmt::emphasis::bold,
                        "***info : complete\n");
                }
                stash_.stop();
                stash_.clear();

                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                ctl_.get_registry().shutdown();
                ctl_.shutdown();

            }
        }
    }
}
