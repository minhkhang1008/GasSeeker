# GasSeeker — Motor wiring

Sơ đồ này chỉ dành cho **ESP32 → 2× TB6612 → 4 motor**.

```mermaid
flowchart LR
    subgraph ESP["ESP32-S3"]
        direction TB
        LEFT["LEFT CONTROL<br/>GPIO5 = PWMA<br/>GPIO6 = AIN1<br/>GPIO7 = AIN2"]
        RIGHT["RIGHT CONTROL<br/>GPIO15 = PWMB<br/>GPIO16 = BIN1<br/>GPIO17 = BIN2"]
        STBY["GPIO18 = STBY"]
    end

    subgraph DRIVER1["TB6612 #1 — PHÍA TRƯỚC"]
        direction TB
        D1A["Kênh A<br/>PWMA / AIN1 / AIN2<br/>AO1-AO2"]
        D1B["Kênh B<br/>PWMB / BIN1 / BIN2<br/>BO1-BO2"]
        D1S["STBY"]
    end

    subgraph DRIVER2["TB6612 #2 — PHÍA SAU"]
        direction TB
        D2A["Kênh A<br/>PWMA / AIN1 / AIN2<br/>AO1-AO2"]
        D2B["Kênh B<br/>PWMB / BIN1 / BIN2<br/>BO1-BO2"]
        D2S["STBY"]
    end

    FL["Motor Front Left (FL)"]
    FR["Motor Front Right (FR)"]
    RL["Motor Rear Left (RL)"]
    RR["Motor Rear Right (RR)"]

    LEFT -->|3 dây dùng chung| D1A
    LEFT -->|3 dây dùng chung| D2A

    RIGHT -->|3 dây dùng chung| D1B
    RIGHT -->|3 dây dùng chung| D2B

    STBY --> D1S
    STBY --> D2S

    D1A -->|AO1 / AO2| FL
    D1B -->|BO1 / BO2| FR
    D2A -->|AO1 / AO2| RL
    D2B -->|BO1 / BO2| RR

    classDef esp fill:#dbeafe,stroke:#2563eb,stroke-width:2px;
    classDef driver fill:#ffedd5,stroke:#ea580c,stroke-width:2px;
    classDef motor fill:#f3e8ff,stroke:#7e22ce,stroke-width:2px;
    class LEFT,RIGHT,STBY esp;
    class D1A,D1B,D1S,D2A,D2B,D2S driver;
    class FL,FR,RL,RR motor;
```

## Bảng nối nhanh

| ESP32 | TB6612 #1 | TB6612 #2 | Ý nghĩa |
|---|---|---|---|
| GPIO5 | PWMA | PWMA | PWM bên trái |
| GPIO6 | AIN1 | AIN1 | chiều bên trái |
| GPIO7 | AIN2 | AIN2 | chiều bên trái |
| GPIO15 | PWMB | PWMB | PWM bên phải |
| GPIO16 | BIN1 | BIN1 | chiều bên phải |
| GPIO17 | BIN2 | BIN2 | chiều bên phải |
| GPIO18 | STBY | STBY | bật/tắt driver |

## Motor nào vào output nào

- TB6612 #1 `AO1/AO2` → Front Left.
- TB6612 #1 `BO1/BO2` → Front Right.
- TB6612 #2 `AO1/AO2` → Rear Left.
- TB6612 #2 `BO1/BO2` → Rear Right.

> Mỗi motor dùng một cầu H riêng. Không ghép hai motor vào chung AO1/AO2 hoặc BO1/BO2.
