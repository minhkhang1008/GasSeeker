# GasSeeker — Power wiring

Dùng sơ đồ này **chỉ để đấu nguồn**. Chưa đấu GPIO ở bước này.

```mermaid
flowchart LR
    BAT["2× 18650 nối tiếp<br/>2S: 6.0–8.4 V"]
    FUSE["Cầu chì 5 A"]
    SW["Công tắc KCD1"]

    BAT -->|BAT+| FUSE --> SW

    subgraph BUCKS["HẠ ÁP"]
        direction TB
        B6["XL4015 #1<br/>chỉnh 6.0 V"]
        B5["LM2596<br/>chỉnh 5.0 V"]
    end

    SW -->|BAT+| B6
    SW -->|BAT+| B5

    V6(("BUS 6 V"))
    V5(("BUS 5 V"))
    V33(("BUS 3V3<br/>từ ESP32"))
    GND(("GND CHUNG"))

    B6 --> V6
    B5 --> V5

    subgraph MOTOR_POWER["NGUỒN MOTOR"]
        direction TB
        TB1["TB6612 #1<br/>VM"]
        TB2["TB6612 #2<br/>VM"]
    end

    V6 --> TB1
    V6 --> TB2

    subgraph FIVEV["NGUỒN 5 V"]
        direction TB
        ESP["ESP32-S3<br/>chân 5V/VIN"]
        MQ3["MQ-3<br/>VCC"]
    end

    V5 --> ESP
    V5 --> MQ3

    ESP -->|chân 3V3| V33

    subgraph THREEV3["NGUỒN 3V3"]
        direction TB
        TB1L["TB6612 #1<br/>VCC logic"]
        TB2L["TB6612 #2<br/>VCC logic"]
        MPU["MPU6050<br/>VCC"]
        LORA["SX1262 / Ra-01SH<br/>VCC"]
        ENC["HC-020K<br/>VCC theo WIRING.md"]
    end

    V33 --> TB1L
    V33 --> TB2L
    V33 --> MPU
    V33 --> LORA
    V33 --> ENC

    BAT -->|BAT−| GND
    B6 -. GND .-> GND
    B5 -. GND .-> GND
    ESP -. GND .-> GND
    MQ3 -. GND .-> GND
    TB1 -. GND .-> GND
    TB2 -. GND .-> GND
    MPU -. GND .-> GND
    LORA -. GND .-> GND
    ENC -. GND .-> GND

    classDef battery fill:#fee2e2,stroke:#dc2626,stroke-width:2px;
    classDef power fill:#ffedd5,stroke:#ea580c,stroke-width:2px;
    classDef logic fill:#dcfce7,stroke:#16a34a,stroke-width:2px;
    classDef ground fill:#e5e7eb,stroke:#111827,stroke-width:2px;
    class BAT,FUSE,SW battery;
    class B6,B5,V6,V5,TB1,TB2,ESP,MQ3 power;
    class V33,TB1L,TB2L,MPU,LORA,ENC logic;
    class GND ground;
```

## Thứ tự đấu

1. `2S 18650 (+) → Fuse 5A → KCD1`.
2. Sau KCD1 chia thành hai nhánh: `XL4015 #1` và `LM2596`.
3. **Chưa cắm module**, chỉnh XL4015 = `6.0 V`, LM2596 = `5.0 V` bằng đồng hồ.
4. 6 V chỉ vào `VM` của hai TB6612.
5. 5 V vào `ESP32 5V/VIN` và `MQ-3 VCC`.
6. Dùng chân `3V3` của ESP32 cấp phần logic/sensor 3V3.
7. Tất cả GND phải nối về cùng một bus GND.

> Không cấp 5 V cho SX1262 / Ra-01SH.
