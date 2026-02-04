package com.davide.SupplyTracker.controller;
import com.davide.SupplyTracker.model.Shipment;
import com.davide.SupplyTracker.model.ShipmentStatus;
import com.davide.SupplyTracker.repository.ShipmentRepository;
import lombok.RequiredArgsConstructor;
import org.springframework.web.bind.annotation.*;
import java.util.List;

@RestController
@RequestMapping("/api/v1/shipments")
@RequiredArgsConstructor
public class ShipmentController {


    private final ShipmentRepository shipmentRepository;

    @PostMapping
    public Shipment createShipment(@RequestBody Shipment shipment) {
        return shipmentRepository.save(shipment);
    }

    @GetMapping
    public List<Shipment> getAllShipments() {
        return shipmentRepository.findAll();
    }

    @DeleteMapping("/{id}")
    public String deleteShipment(@PathVariable Long id) {
        if (!shipmentRepository.existsById(id)) {
            throw new RuntimeException("Can't Delete: ID " + id + " Not Found");
        }
        shipmentRepository.deleteById(id);
        return "Shipping with ID: " + id + " Deleted with success!";
    }
}
