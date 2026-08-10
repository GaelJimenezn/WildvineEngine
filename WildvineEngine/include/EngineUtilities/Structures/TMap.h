/*
 * MIT License
 *
 * Copyright (c) 2024 Roberto Charreton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * In addition, any project or software that uses this library or class must include
 * the following acknowledgment in the credits:
 *
 * "This project uses software developed by Roberto Charreton and Attribute Overload."
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file TMap.h
 * @brief Declara utilidades de TMap usadas por WildvineEngine.
 */
#pragma once
namespace EU {
/**
 * @brief TMap es una clase de mapa (diccionario) dinÃ¡mica para almacenar pares clave-
 * valor.
 *
 * Esta implementaciÃ³n de TMap proporciona una forma sencilla de almacenar y gestionar
 * colecciones de pares clave-valor, con operaciones bÃ¡sicas como agregar, eliminar y
 * acceder a valores.
 * La memoria se gestiona dinÃ¡micamente, aumentando la capacidad del mapa segÃºn sea
 * necesario.
 *
 * @tparam K El tipo de las claves.
 * @tparam V El tipo de los valores.
 */
template <typename K, typename V>
/** @brief Declara class TMap. */
class
TMap {
private:
  /** @brief Declara struct Pair. */
  struct
  Pair {
    K Key;
    V Value;

    Pair()
        : Key()
        , Value() {
    }
    Pair(const K &Key, const V &Value)
        : Key(Key)
        , Value(Value) {
    }
  };

  Pair *Data;      ///< Puntero a la memoria donde se almacenan los pares clave-valor.
  size_t Capacity; ///< Capacidad actual del mapa (nÃºmero de pares que puede almacenar).
  size_t Size;     ///< NÃºmero de pares actualmente en el mapa.

  /**
   * @brief Redimensiona el mapa para tener una nueva capacidad.
   *
   * @param NewCapacity La nueva capacidad del mapa.
   */
  void
  Resize(size_t NewCapacity) {
    /** @brief Crear un nuevo bloque de memoria con la nueva capacidad. */
    Pair *NewData = new Pair[NewCapacity];
    for (size_t i = 0; i < Size; ++i) {
      NewData[i] = Data[i]; ///< Copiar los pares existentes al nuevo bloque de memoria.
    }
    delete[] Data; ///< Liberar la memoria del mapa antiguo.
    /** @brief Actualizar el puntero Data para que apunte al nuevo bloque de memoria. */
    Data = NewData;
    Capacity = NewCapacity; ///< Actualizar la capacidad del mapa.
  }

public:
  /**
   * @brief Constructor por defecto que inicializa el mapa con capacidad y tamaÃ±o cero.
   */
  TMap()
      : Data(nullptr)
      , Capacity(0)
      , Size(0) {
  }

  /**
   * @brief Destructor que libera la memoria asignada al mapa.
   */
  ~TMap() {
    delete[] Data; ///< Liberar la memoria del mapa.
  }

  /**
   * @brief AÃ±ade un nuevo par clave-valor al mapa.
   *
   * @param Key La clave del nuevo par.
   * @param Value El valor del nuevo par.
   */
  void
  Add(const K &Key, const V &Value) {
    for (size_t i = 0; i < Size; ++i) {
      if (Data[i].Key == Key) {
        Data[i].Value = Value; ///< Actualizar el valor si la clave ya existe.
        return;
      }
    }
    if (Size == Capacity) {
      Resize(Capacity == 0 ? 1 : Capacity * 2); ///< Redimensionar si es necesario.
    }
    Data[Size++] = Pair(Key, Value); ///< AÃ±adir el nuevo par y aumentar el tamaÃ±o.
  }

  /**
   * @brief Elimina el par clave-valor en la posiciÃ³n especificada.
   *
   * @param Key La clave del par a eliminar.
   */
  void
  Remove(const K &Key) {
    for (size_t i = 0; i < Size; ++i) {
      if (Data[i].Key == Key) {
        for (size_t j = i; j < Size - 1; ++j) {
          /** @brief Desplazar los pares hacia la izquierda para llenar el hueco. */
          Data[j] = Data[j + 1];
        }
        --Size; ///< Disminuir el tamaÃ±o del mapa.
        return;
      }
    }
    /** @brief Manejar el caso de clave no encontrada. */
    std::cerr << "Key not found" << std::endl;
  }

  /**
   * @brief Sobrecarga del operador [] para acceder a valores por clave.
   *
   * @param Key La clave del valor a acceder.
   * @return Referencia al valor asociado con la clave especificada.
   */
  V &
  operator[](const K &Key) {
    for (size_t i = 0; i < Size; ++i) {
      if (Data[i].Key == Key) {
        return Data[i].Value; ///< Devolver el valor si la clave se encuentra.
      }
    }
    /** @brief Manejar el caso de clave no encontrada. */
    std::cerr << "Key not found" << std::endl;
    exit(1); ///< Salir del programa en caso de error.
  }

  /**
   * @brief VersiÃ³n constante de la sobrecarga del operador [] para acceder a valores por
   * clave.
   *
   * @param Key La clave del valor a acceder.
   * @return Referencia constante al valor asociado con la clave especificada.
   */
  const V &
  operator[](const K &Key) const {
    for (size_t i = 0; i < Size; ++i) {
      if (Data[i].Key == Key) {
        return Data[i].Value; ///< Devolver el valor si la clave se encuentra.
      }
    }
    /** @brief Manejar el caso de clave no encontrada. */
    std::cerr << "Key not found" << std::endl;
    exit(1); ///< Salir del programa en caso de error.
  }

  /**
   * @brief Devuelve el nÃºmero de pares actualmente en el mapa.
   *
   * @return El nÃºmero de pares en el mapa.
   */
  size_t
  Num() const {
    return Size; ///< Devolver el tamaÃ±o actual del mapa.
  }

  /**
   * @brief Devuelve la capacidad actual del mapa.
   *
   * @return La capacidad del mapa.
   */
  size_t
  GetCapacity() const {
    return Capacity; ///< Devolver la capacidad actual del mapa.
  }
};

// EXAMPLE

/*
int main()
{
  TMap<int, std::string> MyMap;
  ///< Crear una instancia de TMap para claves enteras y valores string.
  MyMap.Add(1, "One");  ///< AÃ±adir pares clave-valor al mapa.
  MyMap.Add(2, "Two");
  MyMap.Add(3, "Three");

  MyMap.Remove(2);  ///< Eliminar el par con clave 2.

  std::cout << "Key 1: " << MyMap[1] << std::endl;
  ///< Acceder e imprimir el valor asociado con la clave 1.
  std::cout << "Key 3: " << MyMap[3] << std::endl;
  ///< Acceder e imprimir el valor asociado con la clave 3.

  std::cout << "Size: " << MyMap.Num()
            << ", Capacity: " << MyMap.GetCapacity()
            << std::endl;
  ///< Imprimir el tamaÃ±o y la capacidad del mapa.

  return 0;
}
*/
} // namespace EU
