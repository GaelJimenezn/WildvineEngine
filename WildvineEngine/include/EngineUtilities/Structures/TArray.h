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
 * @file TArray.h
 * @brief Declara utilidades de TArray usadas por WildvineEngine.
 */
#pragma once
namespace EU {
/**
 * @brief TArray es una clase de array dinÃ¡mica para almacenar elementos de tipo T.
 *
 * Esta implementaciÃ³n de TArray proporciona una forma sencilla de almacenar y gestionar
 * colecciones de elementos, con operaciones bÃ¡sicas como agregar, eliminar y acceder a
 * elementos.
 * La memoria se gestiona dinÃ¡micamente, aumentando la capacidad del array segÃºn sea
 * necesario.
 *
 * @tparam T El tipo de elementos almacenados en el array.
 */
template <typename T>
/** @brief Declara class TArray. */
class
TArray {
private:
  T *Data; ///< Puntero a la memoria donde se almacenan los elementos del array.
  /** @brief Capacidad actual del array (nÃºmero de elementos que puede almacenar). */
  size_t Capacity;
  size_t Size; ///< NÃºmero de elementos actualmente en el array.

  /**
   * @brief Redimensiona el array para tener una nueva capacidad.
   *
   * @param NewCapacity La nueva capacidad del array.
   */
  void
  Resize(size_t NewCapacity) {
    /** @brief Crear un nuevo bloque de memoria con la nueva capacidad. */
    T *NewData = new T[NewCapacity];
    for (size_t i = 0; i < Size; ++i) {
      /** @brief Copiar los elementos existentes al nuevo bloque de memoria. */
      NewData[i] = Data[i];
    }
    delete[] Data; ///< Liberar la memoria del array antiguo.
    /** @brief Actualizar el puntero Data para que apunte al nuevo bloque de memoria. */
    Data = NewData;
    Capacity = NewCapacity; ///< Actualizar la capacidad del array.
  }

public:
  /**
   * @brief Constructor por defecto que inicializa el array con capacidad y tamaÃ±o cero.
   */
  TArray()
      : Data(nullptr)
      , Capacity(0)
      , Size(0) {
  }

  /**
   * @brief Destructor que libera la memoria asignada al array.
   */
  ~TArray() {
    delete[] Data; ///< Liberar la memoria del array.
  }

  /**
   * @brief AÃ±ade un nuevo elemento al final del array.
   *
   * @param Element El elemento a aÃ±adir al array.
   */
  void
  Add(const T &Element) {
    if (Size == Capacity) {
      Resize(Capacity == 0 ? 1 : Capacity * 2); ///< Redimensionar si es necesario.
    }
    Data[Size++] = Element; ///< AÃ±adir el nuevo elemento y aumentar el tamaÃ±o.
  }

  /**
   * @brief Elimina el elemento en la posiciÃ³n especificada.
   *
   * @param Index La posiciÃ³n del elemento a eliminar.
   */
  void
  RemoveAt(size_t Index) {
    if (Index >= Size) {
      /** @brief Manejar el caso de Ã­ndice fuera de rango. */
      std::cerr << "Index out of range" << std::endl;
      return;
    }
    for (size_t i = Index; i < Size - 1; ++i) {
      /** @brief Desplazar los elementos hacia la izquierda para llenar el hueco. */
      Data[i] = Data[i + 1];
    }
    --Size; ///< Disminuir el tamaÃ±o del array.
  }

  /**
   * @brief Sobrecarga del operador [] para acceder a elementos por Ã­ndice.
   *
   * @param Index La posiciÃ³n del elemento a acceder.
   * @return Referencia al elemento en la posiciÃ³n especificada.
   */
  T &
  operator[](size_t Index) {
    if (Index >= Size) {
      /** @brief Manejar el caso de Ã­ndice fuera de rango. */
      std::cerr << "Index out of range" << std::endl;
      exit(1); ///< Salir del programa en caso de error.
    }
    return Data[Index]; ///< Devolver el elemento en la posiciÃ³n especificada.
  }

  /**
   * @brief VersiÃ³n constante de la sobrecarga del operador [] para acceder a elementos
   * por Ã­ndice.
   *
   * @param Index La posiciÃ³n del elemento a acceder.
   * @return Referencia constante al elemento en la posiciÃ³n especificada.
   */
  const T &
  operator[](size_t Index) const {
    if (Index >= Size) {
      /** @brief Manejar el caso de Ã­ndice fuera de rango. */
      std::cerr << "Index out of range" << std::endl;
      exit(1); ///< Salir del programa en caso de error.
    }
    return Data[Index]; ///< Devolver el elemento en la posiciÃ³n especificada.
  }

  /**
   * @brief Devuelve el nÃºmero de elementos actualmente en el array.
   *
   * @return El nÃºmero de elementos en el array.
   */
  size_t
  Num() const {
    return Size; ///< Devolver el tamaÃ±o actual del array.
  }

  /**
   * @brief Devuelve la capacidad actual del array.
   *
   * @return La capacidad del array.
   */
  size_t
  GetCapacity() const {
    return Capacity; ///< Devolver la capacidad actual del array.
  }
};

// EXAMPLE

/*
int main() {

  // TArray Example
  TArray<int> MyArray;
  MyArray.Add(1);
  MyArray.Add(2);
  MyArray.Add(3);
  MyArray.Add(4);
  MyArray.Add(5);

  MyArray.Add(6);
  MyArray.RemoveAt(2);

  for (size_t i = 0; i < MyArray.Num(); ++i)
  {
    std::cout << MyArray[i] << " ";
  }
  std::cout << std::endl;

  std::cout << "Size: " << MyArray.Num()
            << ", Capacity: " << MyArray.GetCapacity()
            << std::endl;

  return 0;
}
*/
} // namespace EU
