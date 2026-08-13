# GasSeeker — Sensor wiring

Sơ đồ này chỉ dành cho **HC-020K + MPU6050 + MQ-3 + bumper**.

```mermaid
flowchart LR
    subgraph ESP["ESP32-S3"]
        direction TB
        E1["GPIO1"]
        E4["GPIO4 (ADC)"]
        E8["GPIO8 (SDA)"]
        E9["GPIO9 (SCL)"]
        E38["GPIO38"]
        GND["GND"]
    end

    ENC["HC-020K<br/>OUT"]

    subgraph MQ["MQ-3 ANALOG"]
        direction LR
        AO["MQ-3 AO"]
        R1["10 kΩ"]
        NODE(("ADC node"))
        R2["20 kΩ"]
        MGND["GND"]
        AO --> R1 --> NODE
        NODE --> R2 --> MGND
    end

    subgraph MPU["MPU6050 / GY-521"]
        direction TB
        SDA["SDA"]
        SCL["SCL"]
    end

    subgraph BUMP["V156 bumper"]
        direction TB
        NO["NO"]
        COM["COM"]
    end

    ENC --> E1
    NODE --> E4
    SDA --- E8
    SCL --- E9
    NO --> E38
    COM --> GND
    MGND --> GND

    classDef esp fill:#dbeafe,stroke:#2563eb,stroke-width:2px;
    classDef sensor fill:#dcfce7,stroke:#16a34a,stroke-width:2px;
    classDef analog fill:#f3e8ff,stroke:#7e22ce,stroke-width:2px;
    classDef switch fill:#fee2e2,stroke:#dc2626,stroke-width:2px;
    class E1,E4,E8,E9,E38,GND esp;
    class ENC,SDA,SCL sensor;
    class AO,R1,NODE,R2,MGND analog;
    class NO,COM switch;
```

## Nối từng cụm

### HC-020K
- `OUT → GPIO1`.
- `GPIO2` không dùng.

### MPU6050
- `SDA → GPIO8`.
- `SCL → GPIO9`.
- VCC/GND xem `power.md`.

### MQ-3

```text
MQ-3 AO ── 10 kΩ ──┬── GPIO4
                    │
                  20 kΩ
                    │
                   GND
```

Không nối MQ-3 AO trực tiếp vào GPIO4.

### Bumper V156
- `COM → GND`.
- `NO → GPIO38`.
- `NC` không dùng.
- `GPIO39` không dùng.
