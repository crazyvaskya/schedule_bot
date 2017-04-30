#include <signal.h>

#include <tgbot/Api.h>
#include <tgbot/EventBroadcaster.h>
#include <tgbot/types/Message.h>

#include "sch_bot.h"
#include "utils.h"

void sch_bot::init_commands ()
{
  auto start_handle = [this] (TgBot::Message::Ptr message_in)
  {
    user_id id = message_in->chat->id;
    add_user (id);

    std::string start_answer = "Привет, красавчик! Ты попал к лучшему боту в Телеграме."
                               + sbot::empty_line
                               + "Вызови /help для помощи.";

    send_message (message_in, start_answer);
  };

  auto help_handle = [this] (TgBot::Message::Ptr message_in)
  {
    std::string help_answer = "Этот бот предназначен для оповещения о парах, об их аудиториях или других событий в вашей жизни, которые нельзя пропустить."
                              + sbot::empty_line
                              + "Бот находится в стадии активной разработки, о найденных багах пишите: @rm_bk, @Aenglsmith или @crazyvaskya.";

    send_message (message_in, help_answer);
  };

  auto debug_handle = [this] (TgBot::Message::Ptr message_in)
  {
    if (!is_admin (message_in->chat->id))
      return;

    users[message_in->chat->id].switch_debug ();
    send_message (message_in, "Debug mode enabled. You will be notified about some serious shit.");
  };

  auto kill_handle = [this] (TgBot::Message::Ptr message_in)
  {
    if (!is_admin (message_in->chat->id))
      return;

    send_message_admins (message_in->chat->username + " killed me :(\n\nRestart server now.");
    //raise (SIGINT);
  };

  auto all_handle = [this] (TgBot::Message::Ptr message_in)
  {
    send_message_all (message_in->chat->username + ":\n" + message_in->text);
  };

  auto flood_handle = [this] (TgBot::Message::Ptr message_in)
  {
    send_message_all (message_in->chat->username + " started the /flood testing. Aaaaaa!\n");

    static const int n_flood = 20;

    for (int i = 0; i < n_flood; i++)
      {
        send_message_all (std::to_string (i + 1) + " from " + std::to_string (n_flood) + sbot::empty_line + StringTools::generateRandomString (30));
      }
  };

  auto unknown_command_handle = [this] (TgBot::Message::Ptr message_in)
  {
    send_message (message_in, "Дедушка не понимает тебя");
  };

  auto any_message_handle = [this] (TgBot::Message::Ptr message_in)
  {
    rep->print (rep::message, "User (name = %s, id = %ld) wrote '%s'",
                              message_in->chat->username.c_str (),
                              message_in->chat->id,
                              message_in->text.c_str());

    if (StringTools::startsWith (message_in->text, "/"))
      return;

    send_message (message_in, "Your message is: " + message_in->text);
  };

  getEvents ().onAnyMessage (any_message_handle);
  getEvents ().onUnknownCommand (unknown_command_handle);

  getEvents ().onCommand ("start", start_handle);
  getEvents ().onCommand ("help", help_handle);
  getEvents ().onCommand ("debug", debug_handle);
  getEvents ().onCommand ("kill", kill_handle);
  getEvents ().onCommand ("all", all_handle);
  getEvents ().onCommand ("flood", flood_handle);
}

void sch_bot::init_users ()
{
  // Developer's id as admins
  add_admin (sbot::r_id);
  add_admin (sbot::a_id);
  add_admin (sbot::v_id);

  send_message_admins ("Bot started, current version: " + sbot::version + ". Build time: " + sbot::build_date + " " + sbot::build_time);

  // TODO: implement users import / export
}

void sch_bot::add_user (user_id id)
{
  if (!user_exist (id))
    {
      users.emplace (id, id);
    }
}

void sch_bot::add_admin (user_id id)
{
  add_user (id);
  admins.insert (id);
}

bool sch_bot::user_exist (user_id id) const
{
  return users.find (id) != users.end ();
}

bool sch_bot::is_admin (user_id id) const
{
  return admins.find (id) != admins.end ();
}

void sch_bot::send_message (const TgBot::Message::Ptr message, const std::string &text) const
{
  getApi ().sendMessage (message->chat->id, text);
}

void sch_bot::send_message (user_id id, const std::string &text) const
{
  getApi ().sendMessage (id, text);
}

void sch_bot::send_message_all (const std::string &text) const
{
  for (auto &user_it : users)
    {
      user_id id = user_it.second.get_id ();
      send_message (id, "[Global Message]" + sbot::empty_line + text);
    }
}

void sch_bot::send_message_admins (const std::string &text) const
{
  for (auto &user_it : admins)
    {
      user_id id = user_it;
      send_message (id, "[Admin Report]" + sbot::empty_line + text);
    }
}

void sch_bot::notify_all ()
{
  // TODO: refactor with a notify-queue container to prevent useless work
  for (auto &user_it : users)
    {
      notify_user (user_it.second);
    }
}

void sch_bot::notify_user (user_t &user)
{
  // TODO: implement user events getter and notifying
  pt::ptime time = sbot::curr_time ();

  pt::time_duration from_last_notify = time - user.last_notify;

  if (from_last_notify.total_seconds () < 300)
    return;

  user.last_notify = time;

  std::string message = sbot::program + " every 5 minute notifiyng test. Current time: " + pt::to_simple_string (time)
                        + sbot::empty_line
                        + StringTools::generateRandomString (16);

  send_message (user.get_id (), message);
}
