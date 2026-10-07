# Code map

## Folders

Arrows point from a folder to what it includes; numbers count the #include pairs. Hexagons are outside libraries.

```mermaid
flowchart LR
  n0{{"zlib"}}
  n2["app"] -->|1| n1
  n1["core/include/shop"]
  n2["app"] -->|1| n3
  n3["service/include/shop"]
  n2["app"] -->|1| n4
  n4["storage/include/shop"]
  n5["core/src"] -->|2| n1
  n1["core/include/shop"]
  n3["service/include/shop"] -->|3| n1
  n1["core/include/shop"]
  n6["service/src"] -->|1| n3
  n3["service/include/shop"]
  n4["storage/include/shop"] -->|2| n1
  n1["core/include/shop"]
  n7["storage/src"] -->|2| n4
  n4["storage/include/shop"]
  n7["storage/src"] -->|1| n0
```

## Files

```mermaid
flowchart LR
  subgraph n8["app"]
    n9["main.cpp"]
  end
  subgraph n10["core/include/shop"]
    n11["ConsoleLogger.h"]
    n12["ILogger.h"]
    n13["IStorage.h"]
    n14["Order.h"]
  end
  subgraph n15["core/src"]
    n16["ConsoleLogger.cpp"]
    n17["Order.cpp"]
  end
  subgraph n18["service/include/shop"]
    n19["OrderService.h"]
  end
  subgraph n20["service/src"]
    n21["OrderService.cpp"]
  end
  subgraph n22["storage/include/shop"]
    n23["CompressedStorage.h"]
    n24["FileStorage.h"]
  end
  subgraph n25["storage/src"]
    n26["CompressedStorage.cpp"]
    n27["FileStorage.cpp"]
  end
  n9 --> n11
  n9 --> n19
  n9 --> n23
  n11 --> n12
  n13 --> n14
  n16 --> n11
  n17 --> n14
  n19 --> n12
  n19 --> n13
  n19 --> n14
  n21 --> n19
  n23 --> n24
  n24 --> n12
  n24 --> n13
  n26 --> n23
  n27 --> n24
```

## Where to start reading

Most included (the core everything builds on):

| File | Included by |
| --- | --- |
| `core/include/shop/ILogger.h` | 3 |
| `core/include/shop/Order.h` | 3 |
| `core/include/shop/ConsoleLogger.h` | 2 |
| `core/include/shop/IStorage.h` | 2 |
| `service/include/shop/OrderService.h` | 2 |
| `storage/include/shop/CompressedStorage.h` | 2 |
| `storage/include/shop/FileStorage.h` | 2 |

Most includes (the code that ties things together):

| File | Includes |
| --- | --- |
| `app/main.cpp` | 3 |
| `service/include/shop/OrderService.h` | 3 |
| `storage/include/shop/FileStorage.h` | 2 |
| `core/include/shop/ConsoleLogger.h` | 1 |
| `core/include/shop/IStorage.h` | 1 |
| `core/src/ConsoleLogger.cpp` | 1 |
| `core/src/Order.cpp` | 1 |
| `service/src/OrderService.cpp` | 1 |
| `storage/include/shop/CompressedStorage.h` | 1 |
| `storage/src/CompressedStorage.cpp` | 1 |

## Include cycles

None found.
