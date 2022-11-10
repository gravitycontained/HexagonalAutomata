#include <qpl/qpl.hpp>

using hexagon = qpl::u8;

constexpr auto state_size = 2u;
//constexpr auto state_colors = std::array{ qpl::rgb(20, 20, 20), qpl::rgb::white(), qpl::rgb::red()};
std::array<qpl::rgb, state_size> state_colors;

void make_state_colors() {
	state_colors[0] = qpl::rgb(20, 20, 20);

	auto stop = state_size - 1;
	for (qpl::size i = 0u; i < stop; ++i) {
		auto delta = qpl::f64_cast(i) / stop;
		state_colors[i + 1] = qpl::get_rainbow_color(delta);
	}
}
constexpr auto NEIGHBOURS_RADIUS = 2;
constexpr auto NEIGHBOURS_SIZE = NEIGHBOURS_RADIUS * 6;
using neighbours_uint = qpl::ubit<((qpl::log2(NEIGHBOURS_SIZE)) / 8 + 1) * 8>;

struct rule {
	std::array<std::array<qpl::u8, NEIGHBOURS_SIZE>, state_size> associations;

	void randomize() {
		for (auto& i : this->associations) {
			for (auto& i : i) {
				i = qpl::random(0u, state_size - 1);
			}
		}
	}
	qpl::u8 get(qpl::u8 target, const std::array<neighbours_uint, state_size>& neighbours) const {
		return this->associations[target][neighbours[target]];
	}
};


struct hexagons {
	std::vector<hexagon> hexagons;
	qpl::vec2s dimension;

	hexagon& operator[](qpl::size index) {
		return this->hexagons[index];
	}
	const hexagon& operator[](qpl::size index) const {
		return this->hexagons[index];
	}
	qpl::size size() const {
		return this->hexagons.size();
	}

	auto begin() {
		return this->hexagons.begin();
	}
	auto begin() const {
		return this->hexagons.cbegin();
	}
	auto cbegin() const {
		return this->hexagons.cbegin();
	}
	auto end() {
		return this->hexagons.end();
	}
	auto end() const {
		return this->hexagons.cend();
	}
	auto cend() const {
		return this->hexagons.cend();
	}

	hexagon get(qpl::i32 x, qpl::i32 y) const {
		if (x < 0 || x >= this->dimension.x || y < 0 || y >= this->dimension.y) {
			return hexagon{ 0 };
		}
		else {
			return this->hexagons[y * this->dimension.x + x];
		}
	}

	std::array<neighbours_uint, state_size> count_neighbours(qpl::i32 x, qpl::i32 y) const {
		std::array<neighbours_uint, state_size> result{};

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
						++result[get(cx, cy)];
					}
				}
			}
		}

		return result;
	}
	void test() {
		for (auto& i : this->hexagons) i = 0u;

		auto random = qpl::random(this->dimension - 1);
		auto x = random.x;
		auto y = random.y;
		this->hexagons[y * this->dimension.x + x] = 2u;

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
						this->hexagons[index] = 1u;
					}
				}
			}
		}
	}
	void udpate() {
		auto copy = this->hexagons;
		for (qpl::isize y = 0; y < this->dimension.y; ++y) {
			for (qpl::isize x = 0; x < this->dimension.x; ++x) {
				auto neighbours = this->count_neighbours(x, y);
				auto dead = neighbours[0];
				auto alive = neighbours[1];

				auto& target = copy[y * this->dimension.x + x];
				if (target == 1u) {
					if (alive < 2) {
						target = 0u;
					}
					else if (alive == 2 || alive == 3) {
						target = 1u;
					}
					else if (alive > 3) {
						target = 0u;
					}
				}
				else if (target == 0u) {
					if (alive == 3u) {
						target = 1u;
					}
				}
			}
		}
		this->hexagons = copy;
	}

	void clear() {
		this->hexagons.clear();
	}
	void create(qpl::vec2s size) {
		this->dimension = size;

		this->hexagons.resize(size.x * size.y, 0u);
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

	constexpr static auto use_heatmap = true;
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
	void init() override {

		make_state_colors();
		this->call_on_resize();

		this->hexagons.create({ 400, 400 });

		this->graphic.update(this->hexagons);

	}
	void call_on_resize() override {
		this->view.set_hitbox(*this);
	}
	void updating() override {
		this->update(this->view);

		if (this->update_clock.has_elapsed_reset(this->update_delta)) {
			this->hexagons.udpate();
			this->graphic.update(this->hexagons);
		}

		if (this->event().key_pressed(sf::Keyboard::A)) {
			this->update_delta *= 1.2;
		}
		else if (this->event().key_pressed(sf::Keyboard::D)) {
			this->update_delta *= 1.0 / 1.2;
		}

		if (this->event().key_single_pressed(sf::Keyboard::R)) {
			for (auto& i : this->hexagons) {
				i = qpl::random(0u, state_size - 1);
			}
			this->graphic.update(this->hexagons);
		}
	}
	void drawing() override {
		this->draw(this->graphic, this->view);
	}

	hexagons hexagons;
	hexagons_graphic graphic;
	qsf::view_rectangle view;

	qpl::small_clock update_clock;
	qpl::f64 update_delta = 0.1;
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
