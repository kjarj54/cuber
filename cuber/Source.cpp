#include <SFML/Graphics.hpp>
#include <iostream>

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Image Example");

    // Cargar la textura desde un archivo
    sf::Texture texture;
    if (!texture.loadFromFile("Fondo.png"))
    {
        std::cerr << "Error: No se pudo cargar la imagen desde el archivo." << std::endl;
        return -1; // Manejar el error si la imagen no se carga correctamente
    }

    // Crear un sprite y asignarle la textura cargada
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
        window.draw(sprite); // Dibujar el sprite en la ventana
        window.display();
    }

    return 0;
}
