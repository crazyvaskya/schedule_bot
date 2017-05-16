/*
 * Copyright (c) 2015 Oleg Morozenkov
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <signal.h>
#include <exception>
#include <memory>

#include <tgbot/tgbot.h>

#include "report_system.h"
#include "sch_bot.h"

volatile sig_atomic_t signal_got = 0;

int main (int /* argc */, char *argv[])
{
  // Signal handler
  signal (SIGINT, [] (int s) { printf ("[CRITICAL] Program got SIGINT signal, aborting...\n"); signal_got = s;});

  // Create report system
  report_system &rep = report_system::instance ();
  rep.print (rep::info, "The program '%s' started work", argv[0]);
  rep.print (rep::info, "Build time: %s %s", sbot::build_date.c_str (), sbot::build_time.c_str ());

  // Create and initialize bot
  sch_bot &bot = sch_bot::instance ();

  // Run loop to handle messages
  while (1)
    {
      try
        {
          TgBot::TgLongPoll long_poll (bot);

          rep.print (rep::info, "Long poll started");
          bot.send_message_admins ("Bot started, current version: " + sbot::version + ".\nBuild time: " + sbot::build_date + " " + sbot::build_time);
          while (1)
            {
              long_poll.start ();
              bot.notify_all ();

              if (signal_got)
                return 0;
            }
        }
      catch (std::exception &e)
        {
          rep.print (rep::error, "Exception: %s", e.what ());

          static int count_exceptions = 0;
          if (count_exceptions++ >= sbot::max_exceptions_get)
            throw e;
        }
    }

  return 0;
}
