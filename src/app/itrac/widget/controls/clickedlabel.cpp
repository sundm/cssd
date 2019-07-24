#include "clickedlabel.h"

void ClickedLabel::mouseReleaseEvent(QMouseEvent *evt)
{
	emit Clicked();
}
