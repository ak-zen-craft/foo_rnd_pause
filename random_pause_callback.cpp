#include "pch.h"
#include "foo_rnd_pause.h"

#include <SDK/foobar2000.h>
#include <thread>
#include <chrono>

#include <ctime>
#include <cstdlib>

class random_pause_callback : public play_callback_static {
public:
    unsigned get_flags() override {
        return flag_on_playback_stop | flag_on_playback_starting | flag_on_playback_new_track;
    }

    void on_playback_stop(play_control::t_stop_reason reason) override {
    //    track_counter = 0;  // reset static counter or make track_counter a member variable
    //    FB2K_console_formatter() << "[RandomPause] Playback stopped, track counter reset.";
    }

    // Required to implement these too (even if empty)
//  void on_playback_starting(play_control::t_track_command, bool, abort_callback&) override {}
    void on_playback_starting(play_control::t_track_command, bool) override {}
    
    void on_playback_new_track(metadb_handle_ptr track) override {
        static int track_counter = 0;
        static int fixed_delay_sec = -1;

        track_counter++;
        int nth = foo_rnd_pause::cfg_apply_to_nth_tracks;
        if (nth <= 0) nth = 1;

        if ((track_counter % nth) != 0) {
            FB2K_console_formatter() << "[RandomPause] Skipping pause for track #" << track_counter;
            return;
        }

        int min_sec = foo_rnd_pause::cfg_rnd_min_sec;
        int max_sec = foo_rnd_pause::cfg_rnd_max_sec;
        if (max_sec < min_sec) std::swap(min_sec, max_sec);
        if (min_sec < 0) min_sec = 0;

        int delay_sec = 0;
        if (foo_rnd_pause::cfg_check_fix_pause) {
            if (fixed_delay_sec < 0) {
                fixed_delay_sec = min_sec + (rand() % (max_sec - min_sec + 1));
            }
            delay_sec = fixed_delay_sec;
        }
        else {
            delay_sec = min_sec + (rand() % (max_sec - min_sec + 1));
        }

        // Pause playback
        fb2k::inMainThread([delay_sec]() {
            static_api_ptr_t<playback_control> pb;
            pb->pause(true);
            FB2K_console_formatter() << "[RandomPause] Track #" << track_counter << ", pausing for " << delay_sec << " seconds";
        });

        // Delay in background, then resume on main thread
        std::thread([delay_sec]() {
            std::this_thread::sleep_for(std::chrono::seconds(delay_sec));
            fb2k::inMainThread([] {
                try {
                    static_api_ptr_t<playback_control> pb;

                    if (pb->is_playing() || pb->is_paused()) {
                        pb->pause(false);
                        FB2K_console_formatter() << "[RandomPause] Resuming playback...";
                    }
                    else {
                        FB2K_console_formatter() << "[RandomPause] Playback not active, skipping resume";
                    }
                }
                catch (...) {
                    FB2K_console_formatter() << "[RandomPause] Exception caught during resume";
                }
                });
            }).detach();
    }

    void on_playback_seek(double time) override {}
    void on_playback_pause(bool state) override {}
    void on_playback_edited(metadb_handle_ptr track) override {}
    void on_playback_dynamic_info(const file_info& info) override {}
    void on_playback_dynamic_info_track(const file_info& info) override {}
    void on_volume_change(float new_val) override {}
    void on_playback_time(double time) override {}
};

// Register with foobar
static play_callback_static_factory_t<random_pause_callback> g_random_pause_factory;