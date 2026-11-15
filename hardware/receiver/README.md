# Lightshow Tree Differential Receiver Daughterboard

This KiCad project will hold a small, castellated daughterboard installed on
the pads exposed by removing the factory PAN2110P0AA radio.  Its function is to
receive Falcon differential data feeds and deliver logic-level signals to the
existing STC8H8K64U.

The current KiCad design receives one feed.  The tree now needs two (510
pixels each, see `docs/hardware-plan.md`): channel 1 on GPIO 27 / pad 3 and
channel 2 on GPIO 29 / pad 1.  A future revision duplicates the receiver
circuit for the second channel.  The STC firmware then produces the 30
independent pixel outputs already proven on the factory board.

## Important constraint

The former radio pads provide a particularly useful three-wire interface.  The
factory firmware used the following three pins for the radio protocol, but our
replacement firmware can repurpose any one of them as a receiver-data GPIO:

| STC pin | Radio pin | Factory-radio function | Daughterboard use |
| --- | --- | --- | --- |
| 27 | 3 | data/SDA | channel 1 receiver data |
| 28 | 2 | clock/SCL | spare GPIO |
| 29 | 1 | chip select | channel 2 receiver data |

In addition, the radio footprint exposes its local 3.3 V and ground pads.  The
daughterboard will solder to exactly three factory pads: 3.3 V, ground, and the
chosen GPIO.  The factory radio's other pads are deliberately left unconnected
on the daughterboard.

The established UART pads remain useful only for development/debugging:

| Factory net | STC pin | Development use |
| --- | --- | --- |
| T2 / J1-2 | 19, P3.0 UART RX | ISP/programming |
| T3 / J1-6 | 20, P3.1 UART TX | optional debug output |
| 3.3 V test pad | 15, VCC | alternate measurement point |
| T4 | 17, GND | alternate measurement point |

The castellated edge therefore provides both the mechanical mounting and the
complete run-time electrical interface.  Revision 1 should still include
unpopulated debug pads for its input and ground, rather than a permanent wire
to T2/T3.

## First-revision functional blocks

1. Falcon differential-pair input connector/pads.
2. 3.3 V RS-485/RS-422 receiver with a valid high-speed logic output.
3. Logic connection to STC GPIO 27 (factory radio pin 3), with alternatives
   available on GPIO 28 and GPIO 29 if placement makes another pad preferable.
4. 3.3 V decoupling, input protection/termination option, and ground return.
5. Castellated mechanical edges, sized only after measuring the removed radio
   footprint and available clearances.

## Measurements still required before layout

- Exact PAN2110P0AA footprint dimensions and orientation on the factory PCB.
- Maximum safe daughterboard outline to the left of that footprint.
- Which Falcon differential pair/pairs will be connected and the physical
  connector/cable plan.
- Differential signalling details from the Falcon port: line rate, pair
  assignment, and whether a termination resistor is already present upstream.
