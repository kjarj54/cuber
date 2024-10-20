#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include "Graph.h"
#include "Dijkstra.h"
#include "Floyd.h"
#include <string>

using namespace std;

void loadVerticesFromFile(Graph& graph, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        int id;
        float x, y;
        char dot;
        if (!(iss >> id >> dot >> x >> y)) { break; } // Error al leer la línea
        string nodeName = "Node" + to_string(id);
        graph.addNode(nodeName, x, y);
    }
    file.close();
}

void loadGraphFromFile(Graph& graph, const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        if (line.find("Punto") != string::npos) {
            int id;
            sscanf_s(line.c_str(), "Punto %d:", &id);
            string nodeName = "Node" + to_string(id);

            // Leer vecinos
            getline(file, line);
            istringstream iss(line.substr(line.find(":") + 1));
            string neighbor;
            while (getline(iss, neighbor, ',')) {
                neighbor.erase(remove(neighbor.begin(), neighbor.end(), '['), neighbor.end());
                neighbor.erase(remove(neighbor.begin(), neighbor.end(), ']'), neighbor.end());
                neighbor.erase(remove(neighbor.begin(), neighbor.end(), ' '), neighbor.end());
                int neighborId = stoi(neighbor);
                string neighborName = "Node" + to_string(neighborId);
                graph.addEdge(nodeName, neighborName, 1.0); // Placeholder weight
            }
        }
    }
    file.close();
}

void drawGraph(sf::RenderWindow& window, Graph& graph, sf::Font& font) {
    for ( auto& vertex : graph.getVertices()) {
        Node* node = graph.getNode(vertex);
        auto neighbors = graph.getNeighbors(vertex);

        // Dibujar el nodo
        sf::CircleShape shape(10); // Radio del círculo más grande
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(node->getX(), node->getY());
        window.draw(shape);

        // Dibujar el ID del nodo
        sf::Text text;
        text.setFont(font);
        text.setString(std::to_string(std::stoi(node->getId().substr(4)))); // Extraer el ID numérico
        text.setCharacterSize(14); // Tamaño del texto
        text.setFillColor(sf::Color::Black);
        text.setPosition(node->getX(), node->getY());
        window.draw(text);

        for ( auto& neighbor : neighbors) {
            Node* neighborNode = graph.getNode(neighbor.first);

            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(node->getX(), node->getY()), sf::Color::Red),
                sf::Vertex(sf::Vector2f(neighborNode->getX(), neighborNode->getY()), sf::Color::Red)
            };

            window.draw(line, 2, sf::Lines);

            if (graph.isDirected()) {
                // Dibujar una segunda línea para los bordes bidireccionales
                sf::Vertex reverseLine[] = {
                    sf::Vertex(sf::Vector2f(neighborNode->getX(), neighborNode->getY()), sf::Color::Blue),
                    sf::Vertex(sf::Vector2f(node->getX(), node->getY()), sf::Color::Blue)
                };

                window.draw(reverseLine, 2, sf::Lines);
            }
        }
    }
}

void openMenuWindow()
{
    sf::RenderWindow menuWindow(sf::VideoMode(300, 200), "Menu", sf::Style::Titlebar | sf::Style::Close);

    sf::RectangleShape button1(sf::Vector2f(100, 50));
    button1.setFillColor(sf::Color::Green);
    button1.setPosition(10, 10);

    sf::RectangleShape button2(sf::Vector2f(100, 50));
    button2.setFillColor(sf::Color::Blue);
    button2.setPosition(10, 70);

    sf::RectangleShape toggleButton(sf::Vector2f(100, 50));
    toggleButton.setFillColor(sf::Color::Red);
    toggleButton.setPosition(10, 130);

    bool toggleState = false;

    while (menuWindow.isOpen())
    {
        sf::Event event;
        while (menuWindow.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                menuWindow.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (button1.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    std::cout << "Button 1 pressed" << std::endl;
                }
                if (button2.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    std::cout << "Button 2 pressed" << std::endl;
                }
                if (toggleButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    toggleState = !toggleState;
                    toggleButton.setFillColor(toggleState ? sf::Color::Yellow : sf::Color::Red);
                    std::cout << "Toggle Button state: " << (toggleState ? "On" : "Off") << std::endl;
                }
            }
        }

        menuWindow.clear();

        // Dibujar botones
        menuWindow.draw(button1);
        menuWindow.draw(button2);
        menuWindow.draw(toggleButton);

        menuWindow.display();
    }
}

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

    // Leer los vértices desde el archivo vertices.txt
    loadVerticesFromFile(graph, "vertices.txt");

    // Leer los vecinos desde el archivo puntos.txt
    loadGraphFromFile(graph, "puntos.txt");

    // Cargar la fuente para los números
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        cerr << "Error: No se pudo cargar la fuente arial.ttf." << endl;
        return -1;
    }
    // Crear botón para abrir el menú
    sf::RectangleShape menuButton(sf::Vector2f(100, 50));
    menuButton.setFillColor(sf::Color::Green);
    menuButton.setPosition(fixedWidth - 110, 10); // Posición en la esquina superior derecha



    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (menuButton.getGlobalBounds().contains(event.mouseButton.x, event.mouseButton.y))
                {
                    openMenuWindow();
                }
            }
        }

        window.clear();
        window.draw(sprite);

        drawGraph(window, graph, font);

        // Dibujar botón de menú
        window.draw(menuButton);

        window.display();
    }

    return 0;
}

