#pragma once

#include <QWidget>

namespace Ui {

enum class AnimMode {
	FadeOutIn,
	FadeIn,
	Slide,
	Custom
};

class Loader;

class Source : public QWidget
{
	Q_OBJECT
public:
	Source(QWidget *parent = nullptr);
	~Source() = default;

signals:
	void hidden();
	void shown();

protected:
	virtual void hideAnimated();
	virtual void showAnimated();

	friend class Loader;
};

class Loader : public QWidget
{
	Q_OBJECT

public:
	Loader(QWidget *parent = nullptr);
	~Loader() = default;

	Source *source() const { return _source.get(); };
	void setSource(Source *src);
	void setSourceAnimated(Source *src, AnimMode animMode = AnimMode::FadeOutIn,  int pause = 0);
	
protected:
	void resizeEvent(QResizeEvent *event) override;
	void animate(Source *src, AnimMode animMode);
	//QSize minimumSizeHint() const override;

private:
	std::unique_ptr<Source> _source;
};

////template <class... Args>
////struct type_list {
//	template <std::size_t N, typename... Args>
//	using NthType = typename std::tuple_element<N, std::tuple<Args...>>::type;
////};
//
template<typename T>
Source * create_impl() { return new T; }

template<typename... Types>
class CyclicLoader : public Loader{
public:
	CyclicLoader(int start, QWidget *parent = nullptr)
	: Loader(parent)
	, _cur(start)
	, _size(sizeof...(Types)){
		if (_size)
		Loader::setSource(yield());
	}

	void loadNext() {
		Loader::setSourceAnimated(yield());
	}

	void setSource(Source *src) = delete;
	void setSourceAnimated(Source *src, AnimMode animMode = AnimMode::FadeOutIn, int pause = 0) = delete;
private:

	Source * yield() {
		using create_impl_type = Source*(*)();
		//typedef Source *(*create_impl_type)();
		static constexpr create_impl_type f[] = { create_impl<Types>... };
		Source *s = f[_cur]();
		_cur = (_cur + 1) % _size;
		return s;
	}

	int _cur;
	int _size;
};

} // namespace Ui
