/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <quackmore-ff@yahoo.com> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you 
 * think this stuff is worth it, you can buy me a beer in return. Quackmore
 * ----------------------------------------------------------------------------
 */
#ifndef __ESPBOT_HPP__
#define __ESPBOT_HPP__

#define P_FATAL(...) os_printf(__VA_ARGS__)
#define P_ERROR(...) os_printf(__VA_ARGS__)
#define P_WARN(...) os_printf(__VA_ARGS__)
#define P_INFO(...) os_printf(__VA_ARGS__)
#define P_DEBUG(...) os_printf(__VA_ARGS__)
#define P_TRACE(...) os_printf(__VA_ARGS__)
#define P_ALL(...) os_printf(__VA_ARGS__)


// this function is the SDK callback system_init_done_cb
extern "C"
{
  void espbot_init(void);
}

#define SIG_STAMODE_GOT_IP 0
#define SIG_STAMODE_DISCONNECTED 1
#define SIG_SOFTAPMODE_STACONNECTED 2

class Espbot
{
private:
  char m_name[32];
  // espbot task
  static const int QUEUE_LEN = 4;
  os_event_t *m_queue;
  // heartbeat timer
  static const int HEARTBEAT_PERIOD = 60000;
  os_timer_t m_heartbeat;

protected:
public:
  Espbot(){};
  ~Espbot(){};
  void init(void);
  uint32 get_chip_id(void);
  uint8 get_boot_version(void);
  const char *get_sdk_version(void);
  char *get_version(void);
  char *get_name(void);
  void set_name(char *);
};

#endif
