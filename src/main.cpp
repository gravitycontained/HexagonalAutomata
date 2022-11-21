#include <qpl/qpl.hpp>

constexpr qpl::size max_distint_colors = 30;

namespace info {
	auto state_size = 4u;
	auto neighbours_radius = 4;
	auto hexagons_dimension = qpl::vec(300, 300);
	qpl::f64 empty_rule_chance = 0.5;
	qpl::f64 random_fill_chance = 2.0;
	qpl::size distinct_color_size = 5;
	qpl::size neighbours_size;

	std::vector<qpl::rgb> distinct_colors;
	std::vector<qpl::rgb> state_colors;

	void reshuffle_state_colors() {
		auto stop = info::state_size - 1;
		if (info::distinct_color_size == max_distint_colors) {
			for (qpl::size i = 0u; i < stop; ++i) {
				state_colors[i + 1] = qpl::random_b(0.5) ? qpl::get_random_color() : qpl::get_random_rainbow_color();
			}
		}
		else {
			for (qpl::size i = 0u; i < stop; ++i) {
				state_colors[i + 1] = qpl::random_element(info::distinct_colors);
			}
		}
	}
	void make_state_colors() {
		state_colors.resize(info::state_size);
		state_colors[0] = qpl::rgb(20, 20, 20);

		info::distinct_colors.resize(info::distinct_color_size);
		for (auto& i : info::distinct_colors) {
			i = qpl::random_b(0.5) ? qpl::get_random_color() : qpl::get_random_rainbow_color();
		}

		info::reshuffle_state_colors();
	}
	void calculate_neighbours_size() {
		neighbours_size = qpl::size_cast(qpl::triangle_number(neighbours_radius) * 6 + 1);
	}
}


using neighbours_uint = qpl::u16;
using hexagon = qpl::u8;
constexpr auto undefined = qpl::type_max<hexagon>();


struct rule {
	struct association {
		qpl::vector<hexagon> result_table;
		qpl::size state_index;
	};

	qpl::vector<association> associations;

	void randomize(qpl::f64 ignore_chance) {
		this->associations.resize(info::state_size);
		for (auto& i : this->associations) {
			i.result_table.resize(info::neighbours_size);
			for (auto& i : i.result_table) {
				i = qpl::random(1u, info::state_size - 1);
				if (qpl::random_b(ignore_chance)) {
					i = undefined;
				}
			}
			i.state_index = qpl::random(0u, info::state_size - 1);
		}
		
		if (!this->associations.empty() && !this->associations[0].result_table.empty()) {
			this->associations[0].state_index = qpl::random(1u, info::state_size - 1);
		}
	}
	void mutate() {
		qpl::size mode = qpl::random(0u, 2u);
		switch (mode) {
		case 0u:
			while (true) {
				auto& association = qpl::random_element(this->associations);
				auto before = association.state_index;
				association.state_index = qpl::random(0u, info::state_size - 1);
				if (association.state_index != before) {
					break;
				}
			}
			break;
		default:
			while (true) {
				auto& association = qpl::random_element(this->associations);
				auto& table = qpl::random_element(association.result_table);
				auto before = table;
				table = (mode == 1 ? undefined : qpl::random(1u, info::state_size - 1));
				if (table != before) {
					break;
				}
				association = qpl::random_element(this->associations);
			}
		}
	}
	hexagon get(hexagon target, const std::vector<neighbours_uint>& neighbours) const {
		const auto& association = this->associations[target];
		auto value = association.result_table[neighbours[association.state_index]];
		if (value == undefined) {
			return target;
		}
		return value;
	}

	std::string info_string() const {
		std::ostringstream stream;
		for (qpl::size i = 0u; i < this->associations.size(); ++i) {
			stream << "for current state " << i << " and neighbour state " << (int)this->associations[i].state_index << "\n---";
			for (qpl::size a = 0u; a < this->associations[i].result_table.size(); ++a) {
				if (this->associations[i].result_table[a] != undefined) {
					stream  << a << " -> " << (int)(this->associations[i].result_table[a]) << ", ";
				}
			}
			stream << '\n';
		}
		return stream.str();
	}

	void save(std::string file) const {
		qpl::save_state state;
		state.save(info::state_size);
		state.save(info::neighbours_radius);
		state.save(info::random_fill_chance);
		state.save(info::empty_rule_chance);
		state.save(info::state_colors);
		for (auto& i : this->associations) {
			state.save(i.state_index);
			state.save(i.result_table);
		}
		state.file_save(file);
	}
	void load(std::string file) {
		qpl::save_state state;
		state.file_load(file);

		state.load(info::state_size);
		state.load(info::neighbours_radius);
		state.load(info::random_fill_chance);
		state.load(info::empty_rule_chance);
		info::calculate_neighbours_size();


		info::state_colors.resize(info::state_size);
		state.load(info::state_colors);

		info::distinct_color_size = 0u;
		std::unordered_set<qpl::rgb> seen;
		for (auto& color : info::state_colors) {
			if (seen.find(color) == seen.cend()) {
				info::distinct_colors.push_back(color);
				++info::distinct_color_size;
				seen.insert(color);
			}
		}
		info::distinct_color_size = qpl::min(info::distinct_color_size, max_distint_colors);

		this->associations.resize(info::state_size);
		for (auto& i : this->associations) {
			i.result_table.resize(info::neighbours_size);
			state.load(i.state_index);
			state.load(i.result_table);
		}
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

	std::vector<neighbours_uint> count_neighbours(qpl::i32 x, qpl::i32 y) const {
		std::vector<neighbours_uint> result(info::state_size, 0);

		for (qpl::isize col = 0; col < info::neighbours_radius * 2 + 1; ++col) {
			auto width = (col + info::neighbours_radius + 1);
			if (col > info::neighbours_radius) {
				width = (info::neighbours_radius * 2 + 1) - (col - info::neighbours_radius);
			}

			for (qpl::isize i = 0; i < width; ++i) {

				auto dy = col - info::neighbours_radius;
				auto cy = y + dy;
				auto cx = x + i - info::neighbours_radius + (qpl::abs(dy) / 2);
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
	void udpate() {
		auto copy = this->collection;
		for (qpl::isize y = 0; y < this->dimension.y; ++y) {
			for (qpl::isize x = 0; x < this->dimension.x; ++x) {
				auto neighbours = this->count_neighbours(x, y);

				auto& target = copy[y * this->dimension.x + x];
				target = rule.get(target, neighbours);
			}
		}
		this->collection = copy;
	}

	void clear() {
		this->collection.clear();
	}
	void create(qpl::vec2s size) {
		this->dimension = size;

		this->collection.resize(size.x * size.y);
	}
	void reset() {
		std::fill(this->collection.begin(), this->collection.end(), undefined);
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
			this->va[index * 18 + i].color = info::state_colors[hexagon];
		}
	}

	void update(const hexagons& hexagons) {
		if (!this->created) {
			this->before.clear();
			this->create(hexagons.dimension);
		}

		if (hexagons.dimension != before.dimension) {
			this->create(hexagons.dimension);
			before.dimension = hexagons.dimension;
			before.create(hexagons.dimension);
			before.reset();
		}
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

	void draw(qsf::draw_object& draw) const {
		draw.draw(this->va);
	}
};

struct main_state : qsf::base_state {
	void init() override {
		info::calculate_neighbours_size();
		info::make_state_colors();

		this->print_commands();

		this->call_on_resize();

		this->hexagons.create(info::hexagons_dimension);
		this->graphic.update(this->hexagons);

		this->next_random_rule();

		auto width = 20;
		auto increase = 2;
		qpl::size slider_ctr = 0u;

		this->slider_empty_rule.set_text_font("helvetica");
		this->slider_empty_rule.set_text_character_size(12);
		this->slider_empty_rule.set_text_color(qpl::rgb::grey_shade(150));
		this->slider_empty_rule.set_text_string("empty rule: ");
		this->slider_empty_rule.set_dimensions({ 600, 15 }, { 10, 15 });
		this->slider_empty_rule.set_position({ 20, width + (slider_ctr++) * (width + increase) });
		this->slider_empty_rule.set_range(0.0, 1.0, info::empty_rule_chance);

		this->slider_random_fill = this->slider_empty_rule;
		this->slider_random_fill.set_text_string("random fill: ");
		this->slider_random_fill.set_position({ 20, width + (slider_ctr++) * (width + increase) });
		this->slider_random_fill.set_range(0.0, 6.0, info::random_fill_chance);

		this->slider_neighbour_radius = this->slider_empty_rule;
		this->slider_neighbour_radius.set_text_string("neighbour radius: ");
		this->slider_neighbour_radius.set_position({ 20, width + (slider_ctr++) * (width + increase) });
		this->slider_neighbour_radius.set_range(1, 15, 2);

		this->slider_state_size = this->slider_empty_rule;
		this->slider_state_size.set_text_string("state size: ");
		this->slider_state_size.set_position({ 20, width + (slider_ctr++) * (width + increase) });
		this->slider_state_size.set_range(2, 254, 4);

		this->slider_distinct_colors = this->slider_empty_rule;
		this->slider_distinct_colors.set_text_string("distinct colors: ");
		this->slider_distinct_colors.set_position({ 20, width + (slider_ctr++) * (width + increase) });
		this->slider_distinct_colors.set_range(1, 30, info::distinct_color_size);
		this->slider_distinct_colors.set_text_string_function([](auto s) { return s == max_distint_colors ? qpl::to_string("disabled") : qpl::to_string(s); });

		this->slider_dimension = this->slider_empty_rule;
		this->slider_dimension.set_text_string("dimension: ");
		this->slider_dimension.set_text_string_function([](auto s) {return qpl::to_string(s, " x ", s); });
		this->slider_dimension.set_position({ 20, width + (slider_ctr++) * (width + increase) });
		this->slider_dimension.set_range(10, 1000, 300);

	}

	void print_commands() {
		qpl::println("'A'     - for slower update");
		qpl::println("'C'     - randomize state colors");
		qpl::println("'V'     - reshuffle state colors");
		qpl::println("'D'     - for slower update");
		qpl::println("'L'     - to load random rule from rules/");
		qpl::println("'M'     - mutate current rule");
		qpl::println("'P'     - print current rule");
		qpl::println("'S'     - save current rule to rules/");
		qpl::println("'R'     - randomize state again");
		qpl::println("'X'     - toggle auto update mode");
		qpl::println("'Space' - next random rule");
		qpl::println();
	}
	void call_on_resize() override {
		this->view.set_hitbox(*this);
	}
	void randomize_hexagons() {
		for (auto& i : this->hexagons) {
			i = 0u;
			if (qpl::random_b(1.0 / (std::pow(10, info::random_fill_chance)))) {
				i = qpl::random(0u, info::state_size - 1);
			}
		}
		this->graphic.update(this->hexagons);
	}
	void next_random_rule() {
		this->update_ctr = 0u;
		this->hexagons.rule.randomize(info::empty_rule_chance);
		this->randomize_hexagons();
	}
	void load_random_rule() {
		qpl::filesys::path path = qpl::to_string(qpl::filesys::get_current_location(), "/rules/");
		auto files = path.list_current_directory();

		files.list_keep_where_extension_equals(".dat");

		auto index = qpl::random(0ull, files.size() - 1);
		qpl::println("loading \"", files[index], "\"");
		this->hexagons.rule.load(files[index]);

		this->randomize_hexagons();
		this->update_ctr = 0u;

		this->slider_state_size.set_value(info::state_size);
		this->slider_neighbour_radius.set_value(info::neighbours_radius);
		this->slider_empty_rule.set_value(info::empty_rule_chance);
		this->slider_random_fill.set_value(info::random_fill_chance);
	}
	void updating() override {
		this->update(this->slider_empty_rule);
		this->update(this->slider_random_fill);
		this->update(this->slider_state_size);
		this->update(this->slider_neighbour_radius);
		this->update(this->slider_dimension);
		this->update(this->slider_distinct_colors);

		if (this->slider_empty_rule.value_was_modified()) {
			info::empty_rule_chance = this->slider_empty_rule.get_value();
			this->next_random_rule();
		}
		if (this->slider_random_fill.value_was_modified()) {
			info::random_fill_chance = this->slider_random_fill.get_value();
			this->randomize_hexagons();
		}
		if (this->slider_state_size.value_was_modified()) {
			info::state_size = this->slider_state_size.get_value();
			info::make_state_colors();
			this->next_random_rule();
		}
		if (this->slider_neighbour_radius.value_was_modified()) {
			info::neighbours_radius = this->slider_neighbour_radius.get_value();
			info::calculate_neighbours_size();
			this->next_random_rule();
		}
		if (this->slider_dimension.value_was_modified()) {
			info::hexagons_dimension = qpl::vec2i::filled(this->slider_dimension.get_value());
			this->hexagons.create(info::hexagons_dimension);
			this->graphic.update(this->hexagons);
			this->randomize_hexagons();
		}
		if (this->slider_distinct_colors.value_was_modified()) {
			info::distinct_color_size = this->slider_distinct_colors.get_value();
			info::make_state_colors();
			this->graphic.before.reset();
			this->graphic.update(this->hexagons);
		}

		bool dragging = (this->slider_empty_rule.dragging ||
			this->slider_random_fill.dragging || 
			this->slider_state_size.dragging || 
			this->slider_neighbour_radius.dragging ||
			this->slider_dimension.dragging ||
			this->slider_distinct_colors.dragging);

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
			this->load_random_rule();
		}
		if (this->event().key_single_pressed(sf::Keyboard::S)) {
			auto file = qpl::to_string("rules/", qpl::get_current_time_string_ymdhmsms_compact(), "_rule.dat");
			this->hexagons.rule.save(file);
		}
		if (this->event().key_single_pressed(sf::Keyboard::P)) {
			qpl::println(this->hexagons.rule.info_string(), "\n\n");
		}
		if (this->event().key_single_pressed(sf::Keyboard::C)) {
			info::make_state_colors();
			this->graphic.before.reset();
			this->graphic.update(this->hexagons);
		}
		if (this->event().key_single_pressed(sf::Keyboard::V)) {
			info::reshuffle_state_colors();
			this->graphic.before.reset();
			this->graphic.update(this->hexagons);
		}
		if (this->event().key_single_pressed(sf::Keyboard::R)) {
			this->randomize_hexagons();
		}
		if (this->event().key_pressed(sf::Keyboard::M)) {
			this->hexagons.rule.mutate();
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
		this->draw(this->slider_empty_rule);
		this->draw(this->slider_random_fill);
		this->draw(this->slider_state_size);
		this->draw(this->slider_neighbour_radius);
		this->draw(this->slider_dimension);
		this->draw(this->slider_distinct_colors);
	}

	hexagons hexagons;
	hexagons_graphic graphic;
	qsf::view_rectangle view;

	qsf::slider<qpl::f64> slider_empty_rule;
	qsf::slider<qpl::f64> slider_random_fill;
	qsf::slider<qpl::size> slider_state_size;
	qsf::slider<qpl::size> slider_neighbour_radius;
	qsf::slider<qpl::size> slider_dimension;
	qsf::slider<qpl::size> slider_distinct_colors;
	qpl::size file_index = 0u;

	qpl::small_clock update_clock;
	qpl::f64 update_delta = 0.01;
	qpl::size update_ctr = 0u;
	bool auto_update = false;
};

int main() try {
	qsf::framework framework;
	framework.set_title("QPL");
	framework.add_font("helvetica", "resources/Helvetica.ttf");
	framework.set_dimension({ 1400u, 950u });

	framework.add_state<main_state>();
	framework.game_loop();
}
catch (std::exception& any) {
	qpl::println("caught exception:\n", any.what());
	qpl::system_pause();
}
