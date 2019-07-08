#pragma once

#include <memory>
#include <QString>

namespace Core {

class Launcher {
public:
	Launcher(int argc, char* argv[], const QString &sysVersion);
	~Launcher() = default;

	static std::unique_ptr<Launcher> create(int argc, char *argv[]);

	int start();

	QString sysVersion() const;

private:
	void prepareSettings();
	void initialize();

	int _argc;
	char ** _argv;
	const QString _sysVersion;
};

} // namespace Core
