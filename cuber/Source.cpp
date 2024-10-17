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

    // Leer los vértices desde el archivo
    ifstream verticesFile("vertices.txt");
    if (!verticesFile.is_open()) {
        cerr << "Error: No se pudo abrir el archivo de vértices." << endl;
        return -1;
    }

    string line;
    int nodeId = 1;
    while (getline(verticesFile, line)) {
        istringstream iss(line);
        float x, y;
        if (!(iss >> x >> y)) { break; } // Error al leer la línea
        string nodeName = "Node" + to_string(nodeId++);
        graph.addNode(nodeName, x, y);
    }
    verticesFile.close();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                // Agregar un nuevo nodo en la posición del mouse
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                string newNodeId = "Node" + std::to_string(graph.getVertices().size() + 1);
                graph.addNode(newNodeId, static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
            }
        }

        window.clear();
        window.draw(sprite);

        // Dibujar nodos (ajustados con el tamaño fijo de la ventana)
        for (auto& vertex : graph.getVertices()) {
            Node* node = graph.getNode(vertex);
            sf::CircleShape circle(5);
            circle.setFillColor(sf::Color::Red);
            circle.setPosition(node->getX(), node->getY()); // Las coordenadas permanecen tal como están
            window.draw(circle);
        }

        // Dibujar aristas (ajustadas con el tamaño fijo de la ventana)
        for (auto& vertex : graph.getVertices()) {
            Node* node = graph.getNode(vertex);
            for (auto& neighbor : node->getNeighbors()) {
                Node* neighborNode = graph.getNode(neighbor.first);
                drawArrow(window,
                    sf::Vector2f(node->getX() + 5, node->getY() + 5),
                    sf::Vector2f(neighborNode->getX() + 5, neighborNode->getY() + 5),
                    graph.isDirected());
            }
        }

        window.display();
    }

    return 0;
}
