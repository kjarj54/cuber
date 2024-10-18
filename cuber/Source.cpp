#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Graph.h"
#include "Dijkstra.h"
#include "Floyd.h"
#include <string>

using namespace std;

void drawArrow(sf::RenderWindow& window, sf::Vector2f start, sf::Vector2f end, bool directed) {/*...*/ }

int main()
{
    sf::RenderWindow window;

    // Cargar la imagen de fondo
    sf::Texture texture;
    if (!texture.loadFromFile("Fondo.png"))
    {
        std::cerr << "Error: No se pudo cargar la imagen desde el archivo." << std::endl;
        return -1;
    }

    sf::Vector2u textureSize = texture.getSize();

    // Definir el tamaño fijo de la ventana (puede ser 1280x720 o lo que prefieras)
    const unsigned int fixedWidth = 1280;
    const unsigned int fixedHeight = 720;

    // Crear la ventana con el tamaño fijo
    window.create(sf::VideoMode(fixedWidth, fixedHeight), "SFML Fixed Size Window", sf::Style::Close);

    sf::Sprite sprite;
    sprite.setTexture(texture);

    // Escalar la imagen de fondo para que se ajuste al tamaño de la ventana
    sprite.setScale(
        static_cast<float>(fixedWidth) / textureSize.x,
        static_cast<float>(fixedHeight) / textureSize.y
    );

    Graph graph(true); // true para grafo dirigido

    // Leer los vértices desde el archivo puntos.txt
    ifstream verticesFile("vertices.txt");
    if (!verticesFile.is_open()) {
        cerr << "Error: No se pudo abrir el archivo de vértices." << endl;
        return -1;
    }

    string line;
    while (getline(verticesFile, line)) {
        istringstream iss(line);
        int id;
        float x, y;
        char dot;
        if (!(iss >> id >> dot >> x >> y)) { break; } // Error al leer la línea
        string nodeName = "Node" + to_string(id);
        graph.addNode(nodeName, x, y);
    }
    verticesFile.close();

    // Cargar la fuente para los números
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Error: No se pudo cargar la fuente arial.ttf." << endl;
        return -1;
    }

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

        // Dibujar los vértices
        for (auto& vertex : graph.getVertices()) {
            Node* node = graph.getNode(vertex);
            sf::CircleShape shape(10); // Radio del círculo más grande
            shape.setFillColor(sf::Color::Red);
            shape.setPosition(node->getX(), node->getY());
            window.draw(shape);

            // Dibujar el ID del punto
            sf::Text text;
            text.setFont(font);
            text.setString(to_string(stoi(node->getId().substr(4)))); // Extraer el ID numérico
            text.setCharacterSize(14); // Tamaño del texto
            text.setFillColor(sf::Color::Black);
            text.setPosition(node->getX(), node->getY());
            window.draw(text);
        }

        window.display();
    }

    return 0;
}