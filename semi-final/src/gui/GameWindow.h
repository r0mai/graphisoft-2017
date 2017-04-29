#include "Animation.h"
#include "Game.h"
#include <SFML/Graphics.hpp>


class GameWindow {
public:
	GameWindow(Game& game);
	void Resize();
	void Draw();
	void ProcessEvents();
	bool IsOpen() const;

private:
	void DrawPlayers();
	void DrawDisplays();
	void HandleKeypress(const sf::Event::KeyEvent& ev);
	void HandleMouseMove(const sf::Event::MouseMoveEvent& ev);

	Game& game_;
	sf::RenderWindow window_;
	sf::Clock clock_;
	Point hover_;
	bool invalid_ = true;
};
