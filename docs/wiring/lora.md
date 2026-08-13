# GasSeeker — LoRa wiring

Sơ đồ này chỉ dành cho **ESP32-S3 ↔ SX1262 / Ra-01SH**.

```mermaid
flowchart LR
    subgraph ESP["ESP32-S3"]
        direction TB
        E10["GPIO10"]
        E11["GPIO11"]
        E12["GPIO12"]
        E13["GPIO13"]
        E14["GPIO14"]
        E21["GPIO21"]
        E47["GPIO47"]
    end

    subgraph LORA["SX1262 / Ra-01SH"]
        direction TB
        NSS["NSS / CS"]
        MOSI["MOSI"]
        SCK["SCK"]
        MISO["MISO"]
        RST["RST"]
        BUSY["BUSY"]
        DIO1["DIO1"]
    end

    E10 --> NSS
    E11 --> MOSI
    E12 --> SCK
    E13 --- MISO
    E14 --> RST
    E21 --- BUSY
    E47 --- DIO1

    classDef esp fill:#dbeafe,stroke:#2563eb,stroke-width:2px;
    classDef lora fill:#dcfce7,stroke:#16a34a,stroke-width:2px;
    class E10,E11,E12,E13,E14,E21,E47 esp;
    class NSS,MOSI,SCK,MISO,RST,BUSY,DIO1 lora;
```

## Bảng nối trực tiếp

| ESP32-S3 | SX1262 / Ra-01SH |
|---|---|
| GPIO10 | NSS / CS |
| GPIO11 | MOSI |
| GPIO12 | SCK |
| GPIO13 | MISO |
| GPIO14 | RST |
| GPIO21 | BUSY |
| GPIO47 | DIO1 |

Nguồn LoRa xem `power.md`:
- `VCC → 3V3`.
- `GND → GND chung`.
- Gắn antenna trước khi cấp nguồn.
