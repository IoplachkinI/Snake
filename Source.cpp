#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <chrono>
#include <memory>


class DrawableObj {
public:
	virtual void drawObj() = 0;
};


using namespace std;
using namespace sf;


enum class Cell {
	Empty,
	Head,
	Segment,
	Apple
};


const int maxFPS = 0;
const int milliPerTick = 1000 / 20; //the second number is the tps


const pair<int, int> fieldSize(25, 25);


class Segment : public DrawableObj{
protected:
	pair<int, int> pos;
	RectangleShape rect;
	RenderWindow& window;
	Vector2f scale;

public:

	bool toDraw;

	Segment(RenderWindow& window, pair<int, int> pos, Color color, Color outlineColor, Vector2f scale) :
		window(window),
		pos(pos),
		toDraw(false),
		scale(scale)
	{
		rect.setSize(Vector2f(float(window.getSize().x) / float(fieldSize.first), float(window.getSize().y) / float(fieldSize.second)));
		rect.setPosition(pos.first * rect.getSize().x + rect.getSize().x * (1 - scale.x) / 2.f, pos.second * rect.getSize().y + rect.getSize().y * (1 - scale.y) / 2.f);
		rect.setFillColor(color);
		rect.setOutlineThickness(-rect.getSize().x / 8.f);
		rect.setOutlineColor(outlineColor);
		rect.setScale(scale);
	}

	Segment(const Segment& toCopy) : 
		window(toCopy.window),
		pos(toCopy.pos),
		rect(toCopy.rect),
		scale(toCopy.scale),
		toDraw(false)
	{}

	void move(pair<int, int> delta) {
		rect.move(delta.first * rect.getSize().x + rect.getSize().x * (1 - scale.x) / 2.f, delta.second * rect.getSize().y + rect.getSize().y * (1 - scale.y) / 2.f);
		pos.first += delta.first;
		pos.second += delta.second;
	};

	void setPos(pair<int, int> newPos) {
		rect.setPosition(newPos.first * rect.getSize().x + rect.getSize().x * (1 - scale.x) / 2.f, newPos.second * rect.getSize().y + rect.getSize().y * (1 - scale.y) / 2.f);
		pos.first = newPos.first;
		pos.second = newPos.second;
	}

	pair<int, int> getPos() {
		return pos;
	}

	void drawObj() {
		window.draw(rect);
	}

};


class Head : public Segment {
public:

	Head(RenderWindow& window, pair<int, int> pos, Color color, Color outlineColor, Vector2f scale) :
		Segment(window, pos, color, outlineColor, scale)
	{}

};


class Snake : public DrawableObj {
private:
	vector<shared_ptr<Segment>> body;
	Head head;
	int length;
	vector<vector<Cell>>& field;
	pair<int, int> direction;

	void moveInternal(pair <int, int> newPos, bool toGrow) {
		pair <int, int> prevPos2;
		pair <int, int> prevPos = head.getPos();

		field[prevPos.first][prevPos.second] = Cell::Empty;
		field[newPos.first][newPos.second] = Cell::Head;
		head.setPos(newPos);

		for (auto segment : body) {
			prevPos2 = segment->getPos();
			field[segment->getPos().first][segment->getPos().second] = Cell::Empty;
			field[prevPos.first][prevPos.second] = Cell::Segment;
			segment->setPos(prevPos);
			prevPos = prevPos2;
		}

		if (toGrow) {
			field[prevPos.first][prevPos.second] = Cell::Segment;
			auto newSegment = make_shared<Segment>(*body[body.size() - 1]);
			newSegment->setPos(prevPos);
			body.push_back(newSegment);
			length++;
		}

	}

public:
	Snake(RenderWindow& window, pair<int, int> pos, vector<vector<Cell>>& field, Color color, Color outlineColor, Vector2f scale, int length) :
		head(window, pos, color, outlineColor, scale),
		length(length),
		field(field),
		direction(pair<int, int>(0, 0))
	{
		for (int i = 0; i < length - 1; i++) {
			pos.first++;

			if (pos.first >= fieldSize.first) {
				length = i + 1;
				break;
			}

			auto newSegment = make_shared<Segment>(window, pos, color, outlineColor, scale);
			body.push_back(newSegment);
		}

	}

	void setDirection(pair<int, int> newDirection) {
		if (!((direction.first != 0 && direction.first == -newDirection.first) || (direction.second != 0 && direction.second == -newDirection.second))) {
			direction = newDirection;
		}
	}

	pair <int, int> getHeadPos() {
		return head.getPos();
	}

	Cell move() {
		pair <int, int> newPos (head.getPos().first + direction.first, head.getPos().second + direction.second);

		if (newPos.first >= fieldSize.first) {
			newPos.first = 0;
		}
		else if (newPos.first < 0) {
			newPos.first = fieldSize.first - 1;
		}
		else if (newPos.second >= fieldSize.second) {
			newPos.second = 0;
		}
		else if(newPos.second < 0){
			newPos.second = fieldSize.second - 1;
		}

		switch (field[newPos.first][newPos.second]) {
			case Cell::Segment:
				return Cell::Segment;

			case Cell::Apple:
				moveInternal(newPos, true);
				return Cell::Apple;

			case Cell::Empty:
				moveInternal(newPos, false);
				
		}
		return Cell::Empty;
	}

	void drawObj() {
		head.drawObj();
		for (auto segment : body) {
			segment->drawObj();
		}
	}
};


int play() {
	RenderWindow window(VideoMode(900, 900), "Snake", Style::Close);
	window.setFramerateLimit(maxFPS);

	vector<shared_ptr<DrawableObj>> drawables;
	vector<shared_ptr<Segment>> apples (2);

	vector<vector<Cell>> field(fieldSize.first, vector<Cell>(fieldSize.second));

	for (int i = 0; i < fieldSize.first; i++) {
		for (int j = 0; j < fieldSize.second; j++) {
			field[i][j] = Cell::Empty;
		}
	}

	auto snake = make_shared<Snake>(window, pair<int, int>(12, 12), field, Color(rand() % 256, rand() % 256, rand() % 256), Color(100, 100, 100), Vector2f(0.9f, 0.9f), 5);

	drawables.push_back(snake);

	for (int i = 0; i < int(apples.size()); i++) {
		auto apple = make_shared<Segment>(window, pair <int, int> (-1, -1), Color(200, 0, 0), Color(100, 100, 100), Vector2f(0.7f, 0.7f));
		apples[i] = apple;
	}

	long long now = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();
	long long tickTimer = now;
	long long secTimer = now;
	long long startTime = now;

	int stepInterval = 250;
	int acceleration = 1;
	int stepThreshold = 100;
	int appleProbability = 10; // probability to spawn of 1/<this value> per tick

	int delay = 1000;
	int fps = 0;
	int frames = 0;

	snake->setDirection(pair<int, int>(-1, 0));

	while (window.isOpen())
	{
		Event event;

		window.clear(Color(255, 255, 255));

		while (window.pollEvent(event)) {
			switch (event.type) {
			case Event::Closed:
				window.close();
				return 0;
			case Event::KeyPressed:
				if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W)) {
					snake->setDirection(pair<int, int>(0, -1));
				}
				else if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S)) {
					snake->setDirection(pair<int, int>(0, 1));
				}
				else if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D)) {
					snake->setDirection(pair<int, int>(1, 0));
				}
				else if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A)) {
					snake->setDirection(pair<int, int>(-1, 0));
				}
			}
		}

		now = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now().time_since_epoch()).count();

		if (now - startTime >= delay) {
			while (now - tickTimer >= stepInterval) {

				if (rand() % appleProbability == 0) {
					pair<int, int> applePos(rand() % fieldSize.first, rand() % fieldSize.second);

					int loopCount = 0;
					while (field[applePos.first][applePos.second] != Cell::Empty && loopCount <= fieldSize.first * fieldSize.second) {
						applePos = pair<int, int>(rand() % fieldSize.first, rand() % fieldSize.second);
						loopCount++;
					}

					field[applePos.first][applePos.second] = Cell::Apple;

					for (auto apple : apples) {
						if (!apple->toDraw) {
							apple->toDraw = true;
							apple->setPos(applePos);
							break;
						}
					}
				}

				switch (snake->move()) {
					case Cell::Segment:
						return 1;

					case Cell::Apple:
						for (auto apple : apples) {
							if (apple->getPos() == snake->getHeadPos()) {
								apple->toDraw = false;
							}
						}
						break;
				}

				if (stepInterval - acceleration >= stepThreshold) {
					stepInterval -= acceleration;
				}

				tickTimer = now;
			}

			for (auto drawable : drawables) {
				drawable->drawObj();
			}

			for (auto apple : apples) {
				if (apple->toDraw) {
					apple->drawObj();
				}
			}


		}

		frames++;

		if (now - secTimer >= 1000) {
			fps = frames;
			cout << "fps: " << fps << endl;
			frames = 0;

			secTimer = now;
		}

		window.display();
	}

	return 0;
}


int main() {

	srand(unsigned int(time(0)));

	int exitCode = play();

	while (exitCode == 1) {
		exitCode = play();
	}

	return 0;
}