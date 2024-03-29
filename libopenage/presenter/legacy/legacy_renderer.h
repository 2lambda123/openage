// Copyright 2015-2023 the openage authors. See copying.md for legal info.

#pragma once

#include <SDL2/SDL.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../../coord/tile.h"
#include "../../gamestate/old/game_spec.h"
#include "../../handlers.h"
#include "../../options.h"
#include "legacy.h"

namespace openage {

class GameMain;

/**
 * Options for the renderer.
 * These will be included in the user interface
 * via reflection, so adding new members will
 * always be visible
 *
 * TODO include fog drawing etc
 */
class RenderOptions : public options::OptionNode {
public:
	RenderOptions();

	options::Var<bool> draw_debug;
	options::Var<bool> terrain_blending;
};

/**
 * renders the editor and action views
 */
class LegacyRenderer : DrawHandler {
public:
	LegacyRenderer(LegacyEngine *e, presenter::LegacyDisplay *d);
	~LegacyRenderer();

	bool on_draw() override;

	/**
	 * the game this renderer is using
	 */
	GameMain *game() const;

	/**
	 * GameSpec used by this renderer
	 */
	GameSpec *game_spec() const;

	Texture *gaben;

	RenderOptions settings;

private:
	LegacyEngine *engine;
	presenter::LegacyDisplay *display;
};

} // namespace openage
