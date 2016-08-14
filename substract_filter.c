#include <obs-module.h>
#include <graphics/vec2.h>
#include <graphics/vec4.h>
#include <graphics/image-file.h>
#include <util/dstr.h>

#define SETTING_IMAGE_PATH "image_path"
#define SETTING_STRETCH "stretch"

#define TEXT_IMAGE_PATH obs_module_text("Path")
#define TEXT_STRETCH obs_module_text("StretchImage")
#define TEXT_PATH_IMAGES obs_module_text("BrowsePath.Images")
#define TEXT_PATH_ALL_FILES obs_module_text("BrowsePath.AllFiles")

struct substract_filter_data {
  obs_source_t *context;
  gs_effect_t *effect;

  gs_texture_t *target;
  gs_image_file_t image;
  bool lock_aspect;
};

static const char *substract_filter_get_name(void *unused)
{
  UNUSED_PARAMETER(unused);
  return obs_module_text("Substract Filter");
}

static void substract_filter_update(void *data, obs_data_t *settings)
{
  struct substract_filter_data *filter = data;

  const char *path = obs_data_get_string(settings, SETTING_IMAGE_PATH);
  const char *effect_file = "substract_filter.effect";
  char *effect_path;

  obs_enter_graphics();
  gs_image_file_free(&filter->image);
  obs_leave_graphics();

  gs_image_file_init(&filter->image, path);

  obs_enter_graphics();

  gs_image_file_init_texture(&filter->image);

  filter->target = filter->image.texture;
  filter->lock_aspect = !obs_data_get_bool(settings, SETTING_STRETCH);

  effect_path = obs_module_file(effect_file);
  gs_effect_destroy(filter->effect);
  filter->effect = gs_effect_create_from_file(effect_path, NULL);
  bfree(effect_path);

  obs_leave_graphics();
}


static void substract_filter_defaults(obs_data_t *settings)
{
}

#define IMAGE_FILTER_EXTENSIONS " (*.bmp *.jpg *.jpeg *.png)"

static obs_properties_t *substract_filter_properties(void *data)
{
  obs_properties_t *props = obs_properties_create();
  struct dstr filter_str = {0};

  dstr_copy(&filter_str, TEXT_PATH_IMAGES);
  dstr_cat(&filter_str, IMAGE_FILTER_EXTENSIONS ";;");
	dstr_cat(&filter_str, TEXT_PATH_ALL_FILES);
  dstr_cat(&filter_str, " (*.*)");

  obs_properties_add_path(props, SETTING_IMAGE_PATH, TEXT_IMAGE_PATH, OBS_PATH_FILE, filter_str.array, NULL);

  obs_properties_add_bool(props, SETTING_STRETCH, TEXT_STRETCH);

  dstr_free(&filter_str);

	UNUSED_PARAMETER(data);
  return props;
}

static void *substract_filter_create(obs_data_t *settings, obs_source_t *context)
{
  struct substract_filter_data *filter = bzalloc(sizeof(struct substract_filter_data));
	filter->context = context;

	obs_source_update(context, settings);
  return filter;
}

static void substract_filter_destroy(void *data)
{
  struct substract_filter_data *filter = data;

	obs_enter_graphics();
	gs_effect_destroy(filter->effect);
	gs_image_file_free(&filter->image);
	obs_leave_graphics();

  bfree(filter);
}

static void substract_filter_tick(void *data, float t)
{
}

static void substract_filter_render(void *data, gs_effect_t *effect)
{
  struct substract_filter_data *filter = data;
  obs_source_t *target = obs_filter_get_target(filter->context);
  gs_eparam_t *param;
  struct vec2 add_val = {0};
  struct vec2 mul_val = {1.0f, 1.0f};

  if (!target || !filter->target || !filter->effect) {
		obs_source_skip_video_filter(filter->context);
		return;
  }

  if (filter->lock_aspect) {
		struct vec2 source_size;
		struct vec2 mask_size;
		struct vec2 mask_temp;
		float source_aspect;
		float mask_aspect;
		bool size_to_x;
		float fix;

		source_size.x = (float)obs_source_get_base_width(target);
		source_size.y = (float)obs_source_get_base_height(target);
		mask_size.x = (float)gs_texture_get_width(filter->target);
		mask_size.y = (float)gs_texture_get_height(filter->target);

		source_aspect = source_size.x / source_size.y;
		mask_aspect = mask_size.x / mask_size.y;
		size_to_x = (source_aspect < mask_aspect);

		fix = size_to_x ?
			(source_size.x / mask_size.x) :
			(source_size.y / mask_size.y);

		vec2_mulf(&mask_size, &mask_size, fix);
		vec2_div(&mul_val, &source_size, &mask_size);
		vec2_mulf(&source_size, &source_size, 0.5f);
		vec2_mulf(&mask_temp, &mask_size, 0.5f);
		vec2_sub(&add_val, &source_size, &mask_temp);
		vec2_neg(&add_val, &add_val);
		vec2_div(&add_val, &add_val, &mask_size);
  }

  if (!obs_source_process_filter_begin(filter->context, GS_RGBA,OBS_ALLOW_DIRECT_RENDERING))
		return;

	param = gs_effect_get_param_by_name(filter->effect, "target");
	gs_effect_set_texture(param, filter->target);

	param = gs_effect_get_param_by_name(filter->effect, "mul_val");
	gs_effect_set_vec2(param, &mul_val);

	param = gs_effect_get_param_by_name(filter->effect, "add_val");
	gs_effect_set_vec2(param, &add_val);

	obs_source_process_filter_end(filter->context, filter->effect, 0, 0);

  UNUSED_PARAMETER(effect);
}

struct obs_source_info substract_filter = {
  .id = "substract_filter",
  .type = OBS_SOURCE_TYPE_FILTER,
  .output_flags = OBS_SOURCE_VIDEO,
  .get_name = substract_filter_get_name,
  .create = substract_filter_create,
  .destroy = substract_filter_destroy,
  .update = substract_filter_update,
  .get_defaults = substract_filter_defaults,
  .get_properties = substract_filter_properties,
  .video_tick = substract_filter_tick,
  .video_render = substract_filter_render
};
