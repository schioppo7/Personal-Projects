package com.davide.SupplyTracker.model;

import jakarta.persistence.*;
import jakarta.validation.constraints.NotBlank;
import lombok.*;
import java.util.UUID;

@Entity
@Table(name = "shipments")
@Getter @Setter
@NoArgsConstructor
@AllArgsConstructor
@Builder
public class Shipment { // <--- CAMBIATO DA SupplyTracker A Shipment

    @Id
    @GeneratedValue(strategy = GenerationType.IDENTITY)
    private Long id;

    @Column(unique = true, nullable = false, updatable = false)
    private String trackingNumber;

    @NotBlank(message = "the sender is required")
    private String sender;

    @NotBlank(message = "the recipient is required")
    private String recipient;

    @Enumerated(EnumType.STRING)
    private ShipmentStatus status; // <--- USA ShipmentStatus (Vedi sotto)

    @PrePersist
    private void prePersist() {
        if (this.trackingNumber == null) {
            this.trackingNumber = "CODE = " + UUID.randomUUID().toString().substring(0, 7).toUpperCase();
        }
        if (this.status == null) {
            this.status = ShipmentStatus.CREATED; // <--- USA ShipmentStatus
        }
    }

    public void setStatus(ShipmentStatus status) {
    }
}