#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
 
    sf::RenderWindow window;

  
    sf::Texture texture;
    if (!texture.loadFromFile("Fondo.png"))
    {
        std::cerr << "Error: No se pudo cargar la imagen desde el archivo." << std::endl;
        return -1; 
    }


    sf::Vector2u textureSize = texture.getSize();


    window.create(sf::VideoMode(textureSize.x, textureSize.y), "SFML Image Example");


    sf::Sprite sprite;
    sprite.setTexture(texture);

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear();
        window.draw(sprite); 
        window.display();
    }

    return 0;
}
