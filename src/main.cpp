#include <qpl/qpl.hpp>


constexpr auto state_size = 4u;
constexpr auto NEIGHBOURS_RADIUS = 4;
constexpr auto hexagons_dimension = qpl::vec(600, 600);

constexpr auto NEIGHBOURS_SIZE = qpl::triangle_number(NEIGHBOURS_RADIUS) * 6 + 1;


using neighbours_uint = qpl::ubit<((qpl::log2(NEIGHBOURS_SIZE)) / 8 + 1) * 8>;
using hexagon = qpl::u8;
constexpr auto undefined = qpl::type_max<neighbours_uint>();

std::array<qpl::rgb, state_size> state_colors;

void make_state_colors() {
	state_colors[0] = qpl::rgb(20, 20, 20);

	auto stop = state_size - 1;
	for (qpl::size i = 0u; i < stop; ++i) {
		auto delta = qpl::f64_cast(i) / stop;
		//state_colors[i + 1] = qpl::get_rainbow_color(qpl::random(0.0, 1.0));
		state_colors[i + 1] = qpl::get_random_color();

	}
}
struct rule {
	struct association {
		qpl::array<neighbours_uint, NEIGHBOURS_SIZE> table;
		qpl::size state_index;
	};

	qpl::array<association, state_size> associations;

	void randomize(qpl::f64 ignore_chance) {
		for (auto& i : this->associations) {
			for (auto& i : i.table) {
				i = qpl::random(1u, state_size - 1);
				if (qpl::random_b(ignore_chance)) {
					i = undefined;
				}
			}
			i.state_index = qpl::random(0u, state_size - 1);
		}
	}
	qpl::u8 get(qpl::u8 target, const std::array<neighbours_uint, state_size>& neighbours) const {
		auto value = this->associations[target].table[neighbours[this->associations[target].state_index]];
		if (value == undefined) {
			return target;
		}
		return value;
	}

	std::string info_string() const {
		std::ostringstream stream;
		for (qpl::size i = 0u; i < this->associations.size(); ++i) {
			stream << "for current state " << i << " and neibhour state " << (int)this->associations[i].state_index << "\n---";
			for (qpl::size a = 0u; a < this->associations[i].table.size(); ++a) {
				if (this->associations[i].table[a] != qpl::type_max<neighbours_uint>()) {
					stream  << a << " -> " << (int)(this->associations[i].table[a]) << ", ";
				}
			}
			stream << '\n';
		}
		return stream.str();
	}

	void save(std::string file) const {
		qpl::save_state state;
		state.save(state_size);
		state.save(NEIGHBOURS_RADIUS);
		for (auto& i : this->associations) {
			state.save(i.state_index);
			state.save(i.table);
		}
		state.file_save(file);
	}
	bool load(std::string file) {
		qpl::save_state state;
		state.file_load(file);

		qpl::u32 state_s;
		qpl::i32 neighbour;
		state.load(state_s);
		state.load(neighbour);

		if (state_s != state_size) {
			return false;
		}
		if (neighbour != NEIGHBOURS_RADIUS) {
			return false;
		}

		for (auto& i : this->associations) {
			state.load(i.state_index);
			state.load(i.table);
		}
		return true;
	}
};


struct hexagons {
	std::vector<hexagon> collection;
	qpl::vec2s dimension;
	rule rule;

	hexagons() {
		this->rule.randomize(0.5);
	}
	hexagon& operator[](qpl::size index) {
		return this->collection[index];
	}
	const hexagon& operator[](qpl::size index) const {
		return this->collection[index];
	}
	qpl::size size() const {
		return this->collection.size();
	}

	auto begin() {
		return this->collection.begin();
	}
	auto begin() const {
		return this->collection.cbegin();
	}
	auto cbegin() const {
		return this->collection.cbegin();
	}
	auto end() {
		return this->collection.end();
	}
	auto end() const {
		return this->collection.cend();
	}
	auto cend() const {
		return this->collection.cend();
	}

	hexagon get(qpl::i32 x, qpl::i32 y) const {
		if (x < 0 || x >= this->dimension.x || y < 0 || y >= this->dimension.y) {
			return hexagon{ undefined };
		}
		else {
			return this->collection[y * this->dimension.x + x];
		}
	}

	qpl::array<neighbours_uint, state_size> count_neighbours(qpl::i32 x, qpl::i32 y) const {
		qpl::array<neighbours_uint, state_size> result{};

		for (qpl::isize col = 0; col < NEIGHBOURS_RADIUS * 2 + 1; ++col) {
			auto width = (col + NEIGHBOURS_RADIUS + 1);
			if (col > NEIGHBOURS_RADIUS) {
				width = (NEIGHBOURS_RADIUS * 2 + 1) - (col - NEIGHBOURS_RADIUS);
			}

			for (qpl::isize i = 0; i < width; ++i) {

				auto dy = col - NEIGHBOURS_RADIUS;
				auto cy = y + dy;
				auto cx = x + i - NEIGHBOURS_RADIUS + (qpl::abs(dy) / 2);
				if ((dy % 2) && (y % 2)) cx += 1;

				if (cx >= 0 && cy >= 0 && cx < this->dimension.x && cy < this->dimension.y) {
					if (!(cx == x && cy == y)) {
						auto value = this->get(cx, cy);
						if (value != undefined) {
							++result[value];
						}
					}
				}
			}
		}

		return result;
	}
	void test() {
		for (auto& i : this->collection) i = 0u;

		auto random = qpl::random(this->dimension - 1);
		auto x = random.x;
		auto y = random.y;
		this->collection[y * this->dimension.x + x] = 2u;

		//for (qpl::size n = 1;; ++n) {

		auto n = 3;
		for (qpl::isize col = 0; col < n * 2 + 1; ++col) {
			auto width = (col + n + 1);
			if (col > n) {
				width = (n * 2 + 1) - (col - n);
			}

			for (qpl::isize i = 0; i < width; ++i) {

				auto dy = col - n;
				auto cy = y + dy;
				auto cx = x + i - n + (qpl::abs(dy) / 2);
				if ((dy % 2) && (y % 2)) cx += 1;

				if (cx >= 0 && cy >= 0 && cx < this->dimension.x && cy < this->dimension.y) {
					if (!(cx == x && cy == y)) {
						auto index = cy * this->dimension.x + cx;
						this->collection[index] = 1u;
					}
				}
			}
		}
	}
	void udpate() {
		auto copy = this->collection;
		for (qpl::isize y = 0; y < this->dimension.y; ++y) {
			for (qpl::isize x = 0; x < this->dimension.x; ++x) {
				auto neighbours = this->count_neighbours(x, y);

				auto& target = copy[y * this->dimension.x + x];
				target = rule.get(target, neighbours);

				//auto dead = neighbours[0];
				//auto alive = neighbours[1];
				//if (target == 1u) {
				//	if (alive < 2) {
				//		target = 0u;
				//	}
				//	else if (alive == 2 || alive == 3 || alive == 4) {
				//		target = 1u;
				//	}
				//	else if (alive > 4) {
				//		target = 0u;
				//	}
				//}
				//else if (target == 0u) {
				//	if (alive == 3u) {
				//		target = 1u;
				//	}
				//}
			}
		}
		this->collection = copy;
	}

	void clear() {
		this->collection.clear();
	}
	void create(qpl::vec2s size) {
		this->dimension = size;

		this->collection.resize(size.x * size.y, 0u);
	}
};

struct hexagon_shape {
	std::array<qpl::vec2, 18> vertices;

	constexpr static auto size = qpl::vec(50, 50);
	constexpr static auto hex_scale = qpl::sqrt(3.0) / 2;
	void create(qpl::vector2i position) {
		std::array<qpl::vec2, 6> angles{};

		constexpr auto scale = hexagon_shape::size * qpl::vec(hex_scale, 0.75);
		auto center = position * scale;

		if (position.y % 2) {
			center.x += scale.x / 2;
		}
		for (qpl::size i = 0u; i < 6u; ++i) {
			auto angle = 2 * qpl::pi * ((i + 0.5) / 6.0);
			auto x = std::cos(angle);
			auto y = std::sin(angle);
			//angles[i] = qpl::vec(x, y) * (hexagon_shape::size * 0.48);
			angles[i] = qpl::vec(x, y) * (hexagon_shape::size * 0.5);
		}

		for (qpl::size i = 0u; i < 6u; ++i) {
			vertices[i * 3u + 0] = center;
			for (qpl::size k = 1u; k < 3u; ++k) {
				vertices[i * 3u + k] = angles[(i + k) % angles.size()] + center;
			}
		}
	}
};

struct hexagons_graphic {
	qsf::vertex_array va;
	hexagons before;

	std::vector<qpl::f64> heatmap;

	constexpr static auto use_heatmap = false;
	bool created = false;

	hexagons_graphic() {
		this->va.set_primitive_type(qsf::primitive_type::triangles);
	}

	void create(qpl::vec2s size) {
		this->before.clear();
		auto dim = size.x * size.y;

		hexagon_shape hex;
		this->va.resize(dim * hex.vertices.size());
		this->heatmap.resize(dim, 0.0);

		qpl::size ctr = 0u;
		for (auto vec : size.list_possibilities_range()) {
			hex.create(vec);
			for (qpl::size i = 0u; i < hex.vertices.size(); ++i) {
				this->va[ctr * hex.vertices.size() + i].position = hex.vertices[i];
				this->va[ctr * hex.vertices.size() + i].color = qpl::rgb::white();
			}
			++ctr;
		}
		this->created = true;
	}


	void set(qpl::size index, qpl::rgb color) {
		for (qpl::size i = 0u; i < 18u; ++i) {
			this->va[index * 18 + i].color = color;
		}
	}
	void set(qpl::size index) {
		this->heatmap[index] += 0.1;
	}
	void set(qpl::size index, hexagon hexagon) {
		for (qpl::size i = 0u; i < 18u; ++i) {
			this->va[index * 18 + i].color = state_colors[hexagon];
		}
	}

	void update(const hexagons& hexagons) {
		if (!this->created) {
			this->create(hexagons.dimension);
		}
		if (this->before.size() != hexagons.size()) {
			for (qpl::size i = 0u; i < hexagons.size(); ++i) {
				this->set(i, hexagons[i]);
			}
			this->before = hexagons;
		}
		else {
			for (qpl::size i = 0u; i < hexagons.size(); ++i) {
				if (this->before[i] != hexagons[i]) {
					this->before[i] = hexagons[i];
					if constexpr (use_heatmap) {
						this->set(i);
					}
					else {
						this->set(i, hexagons[i]);
					}
				}
				if constexpr (use_heatmap) {
					this->heatmap[i] *= 0.98;
					this->heatmap[i] = qpl::clamp_0_1(this->heatmap[i]);
					auto color = qpl::rgb::interpolation(std::vector{ qpl::rgb(20, 20, 20), qpl::rgb(100, 100, 255), qpl::rgb::white() }, this->heatmap[i]);
					this->set(i, color);
				}
			}
		}
	}

	void draw(qsf::draw_object& draw) const {
		draw.draw(this->va);
	}
};

struct main_state : qsf::base_state {
	void print_commands() {
		qpl::println("'A'     - for slower update");
		qpl::println("'D'     - for slower update");
		qpl::println("'R'     - randomize state again");
		qpl::println("'C'     - randomize state colors");
		qpl::println("'Space' - next random rule");
		qpl::println("'X'     - toggle auto update mode");
		qpl::println("'L'     - to load random rule from rules/");
		qpl::println("'S'     - save current rule to rules/");
		qpl::println();
	}
	void init() override {

		this->print_commands();

		make_state_colors();

		this->call_on_resize();

		this->hexagons.create(hexagons_dimension);
		this->graphic.update(this->hexagons);

		this->next_random_rule();

		this->slider1.set_dimensions({ 800, 30 }, { 30, 30 });
		this->slider1.set_position({ 30, 30 });
		this->slider1.set_range(0.0, 1.0, 0.5);

		this->slider2 = this->slider1;
		this->slider2.set_position({ 30, 90 });
		this->slider2.set_range(0.0, 6.0, 2.0);
	}
	void call_on_resize() override {
		this->view.set_hitbox(*this);
	}
	void randomize_hexagons() {
		for (auto& i : this->hexagons) {
			i = 0u;
			if (qpl::random_b(1.0 / (std::pow(10, this->slider2.get_value())))) {
				i = qpl::random(0u, state_size - 1);
			}
		}
		this->graphic.update(this->hexagons);
	}
	void next_random_rule() {
		this->update_ctr = 0u;
		this->hexagons.rule.randomize(this->slider1.get_value());
		this->randomize_hexagons();

		qpl::println(this->hexagons.rule.info_string());
		qpl::println();
		qpl::println();
	}
	void updating() override {
		this->update(this->slider1);
		this->update(this->slider2);

		if (this->slider1.value_has_changed()) {
			this->next_random_rule();
		}
		if (this->slider2.value_has_changed()) {
			this->randomize_hexagons();
		}
		bool dragging = (this->slider1.dragging || this->slider2.dragging);

		this->view.allow_dragging = !dragging;
		this->update(this->view);

		if (this->update_clock.has_elapsed_reset(this->update_delta)) {
			++this->update_ctr;
			this->hexagons.udpate();
			this->graphic.update(this->hexagons);
		}

		if (this->event().key_pressed(sf::Keyboard::A)) {
			this->update_delta *= 1.2;
		}
		else if (this->event().key_pressed(sf::Keyboard::D)) {
			this->update_delta *= 1.0 / 1.2;
		}

		if (this->event().key_single_pressed(sf::Keyboard::L)) {
			qpl::filesys::path path = qpl::to_string(qpl::filesys::get_current_location(), "/rules/");
			auto files = path.list_current_directory();

			files.list_keep_where_extension_equals(".dat");

			qpl::filesys::path target;
			for (qpl::size i = 0u; i < 5000; ++i) {
				auto index = qpl::random(0ull, files.size() - 1);
				target = files[index];
				if (this->hexagons.rule.load(target)) {
					break;
				}
			}
			qpl::println("loading \"", target, "\"");
			this->randomize_hexagons();
			this->update_ctr = 0u;
		}
		if (this->event().key_single_pressed(sf::Keyboard::S)) {
			auto file = qpl::to_string("rules/", qpl::get_current_time_string_ymdhmsms_compact(), "_rule.dat");
			this->hexagons.rule.save(file);
		}
		if (this->event().key_single_pressed(sf::Keyboard::C)) {
			make_state_colors();
			this->graphic.before.clear();
			this->graphic.update(this->hexagons);
		}
		if (this->event().key_single_pressed(sf::Keyboard::R)) {
			this->randomize_hexagons();
		}
		if (this->event().key_single_pressed(sf::Keyboard::Space)) {
			this->next_random_rule();
		}
		if (this->event().key_single_pressed(sf::Keyboard::X)) {
			this->auto_update = !this->auto_update;
			qpl::println("auto_update : ", qpl::bool_string(this->auto_update));
		}
		if (this->auto_update && this->update_ctr > 125) {
			this->next_random_rule();
		}
	}
	void drawing() override {
		this->draw(this->graphic, this->view);
		this->draw(this->slider1);
		this->draw(this->slider2);
	}

	hexagons hexagons;
	hexagons_graphic graphic;
	qsf::view_rectangle view;

	qsf::slider<qpl::f64> slider1;
	qsf::slider<qpl::f64> slider2;

	qpl::small_clock update_clock;
	qpl::f64 update_delta = 0.01;
	qpl::size update_ctr = 0u;
	bool auto_update = false;
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
