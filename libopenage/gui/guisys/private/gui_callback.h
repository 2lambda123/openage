// Copyright 2015-2023 the openage authors. See copying.md for legal info.

#pragma once

#include <functional>

#include <QMetaType>
#include <QObject>

namespace qtsdl {

class GuiCallback : public QObject {
	Q_OBJECT

public:
	GuiCallback();
	virtual ~GuiCallback();

signals:
	void process_blocking(const std::function<void()> &f);

public slots:
	void process(const std::function<void()> &f);
};

} // namespace qtsdl
