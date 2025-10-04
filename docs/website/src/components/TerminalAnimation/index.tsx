import React, { useEffect, useRef, useState } from 'react';
import styles from './styles.module.css';

const codeLines = [
  { text: '# grouping / having', type: 'comment' },
  { 
    text: 'SELECT', 
    type: 'keyword',
    continue: [
      { text: ' ', type: 'normal' },
      { text: 'max', type: 'function' },
      { text: '(cpu_usage)', type: 'normal' },
      { text: ' as ', type: 'operator' },
      { text: 'max_cpu', type: 'normal' }
    ]
  },
  { 
    text: 'FROM', 
    type: 'keyword',
    continue: [
      { text: ' ', type: 'normal' },
      { text: 'dashboard', type: 'table-name' }
    ]
  },
  {
    text: 'WHERE',
    type: 'keyword',
    continue: [
      { text: ' time > ', type: 'operator' },
      { text: 'now', type: 'function' },
      { text: '() - ', type: 'operator' },
      { text: '1h', type: 'string' }
    ]
  },
  {
    text: 'GROUP BY',
    type: 'keyword',
    continue: [
      { text: ' host', type: 'normal' }
    ]
  }
];

export default function TerminalAnimation() {
  const [output, setOutput] = useState([]);
  const [showCursor, setShowCursor] = useState(true);
  const lineIndexRef = useRef(0);
  const charIndexRef = useRef(0);
  const currentPartRef = useRef(0);

  useEffect(() => {
    const typingSpeed = 60;
    const lineDelay = 400;
    let timeoutId;

    const typeWriter = () => {
      if (lineIndexRef.current >= codeLines.length) {
        timeoutId = setTimeout(() => {
          setOutput([]);
          lineIndexRef.current = 0;
          charIndexRef.current = 0;
          currentPartRef.current = 0;
          typeWriter();
        }, 3000);
        return;
      }

      const currentLine = codeLines[lineIndexRef.current];
      
      setOutput(prev => {
        const newOutput = [...prev];
        if (!newOutput[lineIndexRef.current]) {
          newOutput[lineIndexRef.current] = { parts: [] };
        }

        const currentLineOutput = newOutput[lineIndexRef.current];

        // 处理主文本
        if (currentPartRef.current === 0) {
          if (charIndexRef.current < currentLine.text.length) {
            currentLineOutput.parts[0] = {
              text: currentLine.text.substring(0, charIndexRef.current + 1),
              type: currentLine.type
            };
            charIndexRef.current++;
            timeoutId = setTimeout(typeWriter, typingSpeed);
          } else {
            currentPartRef.current++;
            charIndexRef.current = 0;
            timeoutId = setTimeout(typeWriter, typingSpeed);
          }
          return newOutput;
        }

        // 处理继续部分
        if (currentLine.continue && currentPartRef.current - 1 < currentLine.continue.length) {
          const continuePart = currentLine.continue[currentPartRef.current - 1];
          
          if (charIndexRef.current < continuePart.text.length) {
            currentLineOutput.parts[currentPartRef.current] = {
              text: continuePart.text.substring(0, charIndexRef.current + 1),
              type: continuePart.type
            };
            charIndexRef.current++;
            timeoutId = setTimeout(typeWriter, typingSpeed);
          } else {
            currentPartRef.current++;
            charIndexRef.current = 0;
            timeoutId = setTimeout(typeWriter, typingSpeed);
          }
          return newOutput;
        }

        // 完成当前行
        lineIndexRef.current++;
        charIndexRef.current = 0;
        currentPartRef.current = 0;
        timeoutId = setTimeout(typeWriter, lineDelay);
        return newOutput;
      });
    };

    timeoutId = setTimeout(typeWriter, 800);

    // 光标闪烁
    const cursorInterval = setInterval(() => {
      setShowCursor(prev => !prev);
    }, 500);

    return () => {
      clearTimeout(timeoutId);
      clearInterval(cursorInterval);
    };
  }, []);

  return (
    <div className={`${styles.terminalWindow} terminal terminal-animation`}>
      <div className={styles.terminalHeader}>
        <div className={`${styles.terminalButton} ${styles.btnClose}`}></div>
        <div className={`${styles.terminalButton} ${styles.btnMinimize}`}></div>
        <div className={`${styles.terminalButton} ${styles.btnMaximize}`}></div>
        <span className={styles.terminalTitle}>EQL Query</span>
      </div>
      <div className={styles.terminalBody}>
        {output.map((line, lineIdx) => (
          <div key={lineIdx} className={styles.codeLine}>
            {line.parts.map((part, partIdx) => (
              <span key={partIdx} className={styles[part.type]}>
                {part.text}
              </span>
            ))}
          </div>
        ))}
        <span className={`${styles.cursor} ${showCursor ? styles.visible : ''}`}></span>
      </div>
    </div>
  );
}

