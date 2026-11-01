// Temporary FreeDOS installation diagnosis console. Uses an isolated HD.
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"strings"

	"gitverse.ru/kshmatov/ai386/internal/audio"
	"gitverse.ru/kshmatov/ai386/internal/config"
	"gitverse.ru/kshmatov/ai386/internal/machine"
	"gitverse.ru/kshmatov/ai386/internal/vga"
)

func screen(m *machine.Machine) string {
	var out strings.Builder
	for row := 0; row < 25; row++ {
		for col := 0; col < 80; col++ {
			b, _ := m.Display().TextCell(row, col)
			if b < 32 || b > 126 {
				b = ' '
			}
			out.WriteByte(b)
		}
		out.WriteByte('\n')
	}
	return out.String()
}

func main() {
	cfg, err := config.Load("build/freedos/freedos-install.toml")
	if err != nil {
		panic(err)
	}
	dir, err := os.MkdirTemp("/tmp", "ai386-freedos-install-")
	if err != nil {
		panic(err)
	}
	hdPath := dir + "/hd.img"
	if existing := os.Getenv("AI386_DEBUG_HD"); existing != "" {
		hdPath = existing
	}
	options := []machine.Option{machine.WithHDPath(hdPath), machine.WithHDSize(cfg.HDSize), machine.WithCDPaths(cfg.CDPaths), machine.WithShared(cfg.Shared), machine.WithBootOrder(cfg.Boot), machine.WithRTCStart("2026-10-11T12:00:00Z")}
	if cfg.SB16 {
		backend := audio.KindOff
		play := os.Getenv("AI386_DEBUG_AUDIO_FILE")
		if play != "" {
			backend = audio.KindFile
		}
		options = append(options, machine.WithSB16(machine.SB16Config{Play: play, Base: cfg.SBBase, IRQ: cfg.SBIrq, DMA8: cfg.SBDMA8, DMA16: cfg.SBDMA16, Backend: backend}))
	}
	hz := cfg.ClockHz
	if v := os.Getenv("AI386_DEBUG_CLOCK_HZ"); v != "" {
		hz, err = strconv.ParseUint(v, 10, 64)
		if err != nil {
			panic(err)
		}
	}
	options = append(options, machine.WithClockHz(hz))
	if os.Getenv("AI386_DEBUG_BOOT_HD") != "" {
		options = append(options, machine.WithBootOrder([]string{"hd"}))
	}
	m, err := machine.New(cfg.RAM, options...)
	if err != nil {
		panic(err)
	}
	defer m.Stop()
	if err := m.Start(); err != nil {
		panic(err)
	}
	fmt.Println("HD", hdPath)
	input := bufio.NewScanner(os.Stdin)
	for input.Scan() {
		var cmd struct {
			Keys       []uint16
			Text       string
			Steps      int
			Until      string
			Require    string
			Reject     string
			PNG        string
			RAM        string
			Quit       bool
			StopAtHalt bool
			Trace      bool
		}
		if err := json.Unmarshal(input.Bytes(), &cmd); err != nil {
			panic(err)
		}
		if cmd.Quit {
			return
		}
		scans := map[byte]uint16{' ': 57, '\n': 28, '/': 53, '\\': 43, ':': 39, '.': 52, '?': 53, '-': 12, '=': 13}
		for i, ch := range []byte("1234567890") {
			scans[ch] = uint16(i + 2)
		}
		for ch, scan := range map[byte]uint16{'a': 30, 'b': 48, 'c': 46, 'd': 32, 'e': 18, 'f': 33, 'g': 34, 'h': 35, 'i': 23, 'j': 36, 'k': 37, 'l': 38, 'm': 50, 'n': 49, 'o': 24, 'p': 25, 'q': 16, 'r': 19, 's': 31, 't': 20, 'u': 22, 'v': 47, 'w': 17, 'x': 45, 'y': 21, 'z': 44} {
			scans[ch] = scan
		}
		for _, ch := range []byte(cmd.Text) {
			shifted := ch == ':' || ch == '?'
			if shifted {
				m.KeyEvent(42, true)
			}
			m.KeyEvent(scans[ch], true)
			m.KeyEvent(scans[ch], false)
			for i := 0; i < 1000000 && m.Input().KeyLen() > 0; i++ {
				m.Step()
			}
			if shifted {
				m.KeyEvent(42, false)
			}
		}
		for _, key := range cmd.Keys {
			m.KeyEvent(key, true)
			m.KeyEvent(key, false)
		}
		var history [128]string
		faults := 0
		entrySaved := false
		var previousCR2 uint32
		var previousException uint32
		if cmd.Trace {
			os.WriteFile("/tmp/ai386-freedos-jemm-faults.txt", nil, 0600)
		}
		for i := 0; i < cmd.Steps; i++ {
			if cmd.Trace {
				c := m.CPU()
				pc := c.SegBase(1) + c.EIP()
				var code [8]byte
				if pc+8 < uint32(len(m.RAM())) {
					copy(code[:], m.RAM()[pc:pc+8])
				}
				vec, errCode := c.LastException()
				if pc == 0x1109f9 && !entrySaved {
					entrySaved = true
					os.WriteFile("/tmp/ai386-freedos-jemm-entry.ram", m.RAM(), 0600)
				}
				history[i%128] = fmt.Sprintf("PC=%x exception=%d/%x regs=%x flags=%x CR2=%x CR3=%x segbases=%x code=%x", pc, vec, errCode, c.Registers(), c.EFLAGS(), c.CR2(), c.CR3(), []uint32{c.SegBase(0), c.SegBase(1), c.SegBase(2), c.SegBase(3), c.SegBase(4), c.SegBase(5)}, code)
			}

			m.Step()
			if cmd.Trace {
				vec, _ := m.CPU().LastException()
				cr2 := m.CPU().CR2()
				if vec == 14 && (previousException != 14 || previousCR2 != cr2) && faults < 128 {
					faults++
					f, _ := os.OpenFile("/tmp/ai386-freedos-jemm-faults.txt", os.O_WRONLY|os.O_APPEND, 0600)
					fmt.Fprintf(f, "FAULT %d CR2=%x\n", faults, cr2)
					if faults == 1 {
						os.WriteFile("/tmp/ai386-freedos-jemm-first.ram", m.RAM(), 0600)
					}
					for j := i - 127; j <= i; j++ {
						if j >= 0 {
							fmt.Fprintln(f, history[j%128])
						}
					}
					f.Close()
				}
				previousException = vec
				previousCR2 = cr2
			}
			if cmd.StopAtHalt && m.CPU().Halted() && (cmd.Until == "" || strings.Contains(screen(m), cmd.Until)) {
				break
			}
			if !cmd.StopAtHalt && (i+1)%100000 == 0 && cmd.Until != "" && strings.Contains(screen(m), cmd.Until) {
				break
			}
			if m.CPU().Halted() && !m.CPU().IF() {
				if cmd.Trace {
					f, _ := os.Create("/tmp/ai386-freedos-jemm-trace.txt")
					for j := i + 1; j <= i+128; j++ {
						fmt.Fprintln(f, history[j%128])
					}
					f.Close()
					os.WriteFile("/tmp/ai386-freedos-jemm-trace.ram", m.RAM(), 0600)
				}
				break
			}
		}
		if cmd.RAM != "" {
			if err := os.WriteFile(cmd.RAM, m.RAM(), 0600); err != nil {
				panic(err)
			}
		}
		fmt.Printf("FLAGS=%x CR2=%x CR3=%x regs=%x BDA ticks=%x\n", m.CPU().EFLAGS(), m.CPU().CR2(), m.CPU().CR3(), m.CPU().Registers(), m.RAM()[0x46c:0x470])
		vec, code := m.CPU().LastException()
		fmt.Printf("EIP=%x CSbase=%x CR0=%x exception=%d/%x halted=%v\n%s", m.CPU().EIP(), m.CPU().SegBase(1), m.CPU().CR0(), vec, code, m.CPU().Halted(), screen(m))
		if cmd.PNG != "" {
			if err := vga.SavePNG(cmd.PNG, m.Display().Frame()); err != nil {
				panic(err)
			}
		}
		if cmd.Require != "" && !strings.Contains(screen(m), cmd.Require) {
			panic("required screen missing: " + cmd.Require)
		}
		if cmd.Reject != "" && strings.Contains(screen(m), cmd.Reject) {
			fmt.Fprintln(os.Stderr, "FAIL: guest displayed "+cmd.Reject)
			os.Exit(1)
		}
	}
	if err := input.Err(); err != nil {
		panic(err)
	}
}
