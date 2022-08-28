#pragma once

#include "image_buffer.h"

/**
 * \brief Render's a color gradient (red across x, green across y, blue 25%).
 *        Useful for checking that our SDL textures have the right image format,
 *        and also just looks nice.
 * \param buffer The buffer to render into.
 */
void render_gradient_pattern(image_buffer_t& buffer);
