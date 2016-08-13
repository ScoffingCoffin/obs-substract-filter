#include <obs-module.h>

OBS_DECLARE_MODULE()

OBS_MODULE_USE_DEFAULT_LOCALE("obs-substract", "en-US")

extern struct obs_source_info substract_filter;

bool obs_module_load(void)
{
  obs_register_source(&substract_filter);
  return true;
}
