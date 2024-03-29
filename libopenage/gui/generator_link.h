// Copyright 2015-2023 the openage authors. See copying.md for legal info.

#pragma once

#include "gamestate/old/generator.h"

#include "guisys/link/gui_item_list_model.h"
#include "guisys/link/gui_list_model.h"

namespace openage {
class Generator;
namespace gui {
class GeneratorLink;
}
} // namespace openage

namespace qtsdl {
template <>
struct Wrap<openage::Generator> {
	using Type = openage::gui::GeneratorLink;
};

template <>
struct Unwrap<openage::gui::GeneratorLink> {
	using Type = openage::Generator;
};

} // namespace qtsdl

namespace openage {
namespace gui {

class GeneratorLink : public qtsdl::GuiListModel
	, public qtsdl::GuiItemListModel<GeneratorLink> {
	Q_OBJECT

public:
	GeneratorLink(QObject *parent = nullptr);
	virtual ~GeneratorLink();
};

} // namespace gui
} // namespace openage
