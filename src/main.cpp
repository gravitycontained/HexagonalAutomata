#include <qpl/qpl.hpp>

struct main_state : qsf::base_state {
	void init() override {

	}
	void updating() override {

	}
	void drawing() override {

	}
};

int main() try {
	qsf::framework framework;
	framework.set_title("QPL");
	framework.set_dimension({ 1400u, 950u });

	framework.add_state<main_state>();
	framework.game_loop();
}
catch (std::exception& any) {
	qpl::println("caught exception:\n", any.what());
	qpl::system_pause();
}
