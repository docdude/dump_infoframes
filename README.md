dump_infoframes
=======

`dump_infoframes` is a tool for dumping Raspberry Pi4 HDMI Ram Packet memory from
`/dev/mem`, similar to `devmem2`. It is hard-coded to dump Pi4 Vc5 Ram Packet memory addresses.
The addresses can be changed for other pi's by getting the register addresses from dts files specific
for the Pi model.

Building
--------
Download. Run `make`. That's all.

Limitations
-----------
Obviously, `dump_infoframes` will only work with kernels that have `/dev/mem` enabled.
 