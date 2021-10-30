#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <ctime>
#include <iostream>
/*
windres src\rsrc.rc -O coff -o rsrc.res
windres src\rsrc.rc -O coff -o rsrc.res
g++ -LC:\SFML-2.5.1\lib -o main.exe -lmingw32 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-main -mwindows
*/


/*
g++ -IC:\SFML-2.5.1\include -c main.cpp
g++ -LC:\SFML-2.5.1\lib .\main.o -o main.exe -lmingw32 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-main -mwindows src\rsrc.res
*/
struct Text
{
  sf::RenderWindow& window;
  sf::Text txt;
  sf::Font font;
  int x;
  int y;

  Text(sf::RenderWindow& window, std::string fontPath = "src/CozetteVector.ttf") :
    window(window)
  {
    if (!font.loadFromFile(fontPath)) {
      std::cout << "error: loading font was not possible";
    }
    txt.setFont(font);
    txt.setString("text");
  }

  void draw(std::string str, int fontSize, sf::Color fontColor, int x, int y,
  bool setOrigin = false, sf::Vector2f origin = sf::Vector2f (0, 0))
  {
    if(setOrigin) {
      txt.setOrigin(origin.x, origin.y);
    } else {
      sf::FloatRect bounds = txt.getLocalBounds();
      txt.setOrigin(bounds.left + bounds.width / 2, bounds.top + bounds.height / 2);
    }
    txt.setString(str);
    txt.setFillColor(fontColor);
    txt.setCharacterSize(fontSize);
    txt.setPosition(x, y);
    window.draw(txt);
  }
};

struct Bubble
{
  static const int radius = 20;
  float x;
  float y;
  char letter;
  float speed;
  bool alive = true;

  Bubble(float x, float y, char letter, float speed = 1) :
    x (x), y (y), letter (letter), speed (speed) {
  }

  void update() {
    y += speed;
  }

  void draw(sf::RenderWindow& window)
  {
    static Text txt(window);
    static sf::CircleShape circle(Bubble::radius);

    circle.setPosition(x, y);
    circle.setFillColor(sf::Color::White);
    window.draw(circle);

    txt.draw(std::string(1, letter), Bubble::radius, sf::Color::Black, x + Bubble::radius, y + Bubble::radius);

    update();
  }
};

struct Board
{
  sf::RenderWindow& window;
  std::vector<Bubble> bubbles;
  std::vector<sf::RectangleShape> lifePoints;
  int hits = 0;
  int misses = 5;
  char input;
  bool hitKey = false;
  Text txt;

  Board(sf::RenderWindow& window) :
    window(window),
    txt(window)
  {
    txt.txt.setCharacterSize(20);
    sf::FloatRect bounds = txt.txt.getLocalBounds();
    sf::RectangleShape temp(sf::Vector2f(
      bounds.height, bounds.height));
    for (int i = 0; i < misses; i++)
    {
      lifePoints.push_back(sf::RectangleShape(temp));
    }
  }

  void addBubble() {
    char letter = 'A' + rand() % 26;
    float speed = (1 + rand() % 700) / 100;
    Bubble temp(rand() % (int)window.getSize().x, -2 * Bubble::radius, letter, speed);
    bubbles.push_back(temp);
  }

  void draw() {
    for (Bubble& b : bubbles) {
      b.draw(window);
      if (hitKey && b.letter == input)
      {
        b.alive = false;
        hits+=10;
      }
      else if (b.y >= (float)window.getSize().y) {
        b.alive = false;
        if (lifePoints.size() > 0) {
          lifePoints.pop_back();
          misses--;
        }
      }
    }

    hitKey = false;

    txt.draw("Life: ", 20, sf::Color::White, 10, 10, true);

    sf::FloatRect bounds = txt.txt.getLocalBounds();

    float gap = bounds.height + 5;

    txt.draw("Score: " + std::to_string(hits), 20, sf::Color::White, 10, 10 + gap, true);
    txt.draw("Active bubbles: " + std::to_string(bubbles.size()), 20, sf::Color::White, 10, 10 + gap * 2, true);

    for (int i = 0; i < misses; i++) {
      lifePoints[i].setPosition(10 + bounds.width + 5, 10);
      lifePoints[i].move((gap) * i, 0);
      window.draw(lifePoints[i]);
    }
  }

  void remove()
  {
    std::vector<Bubble> temp;
    for (Bubble& b : bubbles)
    {
      if (b.alive)
      {
        temp.push_back(b);
      }
    }
    bubbles = temp;
  }

  void update()
  {
    remove();

    static const int bubbleTimeTarget = 50;
    static int bubbleTimer = bubbleTimeTarget;

    bubbleTimer--;
    if (bubbleTimer == 0)
    {
      addBubble();
      bubbleTimer = bubbleTimeTarget;
    }
  }
};

struct EndScreen {
  sf::RenderWindow& window;
  sf::RectangleShape sqr;
  bool sqrSet = false;
  bool done = false;
  std::string targetString = "game over";
  std::string currentString = "";
  Text txt;
  EndScreen(sf::RenderWindow& window) :
    window(window),
    sqr(sf::Vector2f((float)window.getSize().x, (float)window.getSize().y)),
    txt(window)
  {
    sqr.setFillColor(sf::Color::White);
    sqr.setPosition(0, (float)window.getSize().y * -1.0);
  }
  void update()
  {

    if (sqr.getPosition().y >= 0)
    {
      sqrSet = true;
    }
    else
    {
      sqr.move(0, 3);
    }
  }
  void draw()
  {
    static const int textTimeTarget = 10;
    static int textTimer = 0;
    update();

    if (sqrSet)
    {
      if (currentString != targetString)
      {

        textTimer--;
        if (textTimer <= 0)
        {
          currentString += targetString[currentString.length()];
          textTimer = textTimeTarget;
        }
      }
      else
      {
        done = true;
      }
    }
    window.draw(sqr);
    txt.draw(currentString, 40, sf::Color::Black, (float)window.getSize().x / 2, (float)window.getSize().y / 2);
  }
};
struct Game
{
  sf::RenderWindow window;
  Board board;
 // auto drawFunc;
  EndScreen endScreen;
  Game() :
    window(sf::VideoMode(840, 600),
      "Bubbles"),
    board(window),
    endScreen(window)
  {
    sf::Image icon;
    icon.loadFromFile("src\\icon.png");  
    window.setIcon(256, 256, icon.getPixelsPtr());
    window.setFramerateLimit(50);
  }

  void handleEvents()
  {
    sf::Event evnt;
    while (window.pollEvent(evnt))
    {
      if (evnt.type == sf::Event::Closed)
      {
        window.close();
      }
      if (evnt.type == sf::Event::Resized) {
        sf::FloatRect newView(0,0,evnt.size.width, evnt.size.height);
        window.setView(sf::View(newView));
        endScreen.sqr.setSize(sf::Vector2f(evnt.size.width, evnt.size.height));
      }
      if (evnt.type == sf::Event::TextEntered)
      {
        char letter = static_cast<char>(evnt.text.unicode);
        letter = toupper(letter);
        board.input = letter;
        board.hitKey = true;
      }
    }
  }

  void draw()
  {
    board.update();
    window.clear();
    board.draw();
    if (board.misses <= 0)
    {
      endScreen.draw();
      if (endScreen.done) {

      }
    }
    window.display();
  }

  void drawEnd() {
    endScreen.draw();
  }

  void mainLoop()
  {
    srand(time(NULL));
    while (window.isOpen())
    {
      handleEvents();
      draw();
    }
  }
};

int main()
{
  Game game;
  game.mainLoop();
}
