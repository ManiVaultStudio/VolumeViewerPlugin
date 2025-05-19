#pragma once

#include <QKeyEvent>
#include <vector>
using namespace std;

enum selectState {
	Idle, CubicSelect, SphereSelect
};

class Interaction3D {
	vector<int> keyState;
	selectState state;
public:
	Interaction3D();
	void readKeyEvent(const QKeyEvent &event, const bool & keyDown) {
		if (event.key() == 'c') {
			state = keyDown ? selectState::CubicSelect : selectState::Idle;
		}
	}
};