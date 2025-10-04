import type {ReactNode} from 'react';
import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';
import Heading from '@theme/Heading';
import TerminalAnimation from '@site/src/components/TerminalAnimation';

import styles from './index.module.css';

function TechIcon({children}: {children: ReactNode}) {
  return (
    <div className={clsx(styles.techIcon, 'tech-icon')}>
      {children}
    </div>
  );
}

function FloatingElements() {
  return (
    <div className={styles.floatingElements}>
      <div className={styles.floatingDot} style={{top: '20%', left: '10%', animationDelay: '0s'}}></div>
      <div className={styles.floatingDot} style={{top: '60%', right: '15%', animationDelay: '2s'}}></div>
      <div className={styles.floatingDot} style={{bottom: '30%', left: '20%', animationDelay: '4s'}}></div>
      <div className={styles.floatingRing} style={{top: '15%', right: '20%', animationDelay: '1s'}}></div>
      <div className={styles.floatingRing} style={{bottom: '20%', right: '10%', animationDelay: '3s'}}></div>
    </div>
  );
}

function HomepageHeader() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <header className={clsx('hero hero--primary', styles.heroBanner)}>
      <FloatingElements />
      <div className="container">
        <div className={styles.heroContent}>
          <TechIcon>
            <svg width="64" height="64" viewBox="0 0 64 64" fill="none" xmlns="http://www.w3.org/2000/svg">
              {/* 外层发光环 */}
              <circle cx="32" cy="32" r="30" fill="url(#outerGlow)" fillOpacity="0.1"/>
              
              {/* 主体六边形容器 */}
              <path d="M32 6L50 16V48L32 58L14 48V16L32 6Z" fill="url(#hexagonFill)" fillOpacity="0.15" stroke="url(#hexagonStroke)" strokeWidth="2"/>
              
              {/* 内部数据层 */}
              <ellipse cx="32" cy="20" rx="12" ry="4" fill="url(#dataLayer1)" fillOpacity="0.8"/>
              <ellipse cx="32" cy="32" rx="14" ry="5" fill="url(#dataLayer2)" fillOpacity="0.6"/>
              <ellipse cx="32" cy="44" rx="10" ry="3" fill="url(#dataLayer3)" fillOpacity="0.9"/>
              
              {/* 连接线 */}
              <path d="M20 20L20 44M44 20L44 44" stroke="url(#connectionLines)" strokeWidth="2" strokeOpacity="0.7"/>
              <path d="M18 32L46 32" stroke="url(#connectionLines)" strokeWidth="1.5" strokeOpacity="0.5"/>
              
              {/* 中心核心 */}
              <circle cx="32" cy="32" r="6" fill="url(#coreGradient)" stroke="url(#coreStroke)" strokeWidth="1.5"/>
              
              {/* 数据流动效果 */}
              <path d="M26 28L38 28M26 36L38 36" stroke="url(#dataFlow)" strokeWidth="1" strokeLinecap="round" strokeOpacity="0.8">
                <animate attributeName="stroke-dasharray" values="0 10;5 5;10 0;5 5;0 10" dur="2s" repeatCount="indefinite"/>
              </path>
              
              <defs>
                <radialGradient id="outerGlow" cx="32" cy="32" r="30" gradientUnits="userSpaceOnUse">
                  <stop stopColor="#06b6d4" stopOpacity="0.3"/>
                  <stop offset="0.7" stopColor="#3b82f6" stopOpacity="0.1"/>
                  <stop offset="1" stopColor="#8b5cf6" stopOpacity="0"/>
                </radialGradient>
                
                <linearGradient id="hexagonFill" x1="14" y1="6" x2="50" y2="58" gradientUnits="userSpaceOnUse">
                  <stop stopColor="#1e40af"/>
                  <stop offset="0.5" stopColor="#3b82f6"/>
                  <stop offset="1" stopColor="#6366f1"/>
                </linearGradient>
                
                <linearGradient id="hexagonStroke" x1="14" y1="6" x2="50" y2="58" gradientUnits="userSpaceOnUse">
                  <stop stopColor="#60a5fa"/>
                  <stop offset="0.5" stopColor="#06b6d4"/>
                  <stop offset="1" stopColor="#a78bfa"/>
                </linearGradient>
                
                <linearGradient id="dataLayer1" x1="20" y1="20" x2="44" y2="20" gradientUnits="userSpaceOnUse">
                  <stop stopColor="#06b6d4"/>
                  <stop offset="1" stopColor="#3b82f6"/>
                </linearGradient>
                
                <linearGradient id="dataLayer2" x1="18" y1="32" x2="46" y2="32" gradientUnits="userSpaceOnUse">
                  <stop stopColor="#3b82f6"/>
                  <stop offset="1" stopColor="#6366f1"/>
                </linearGradient>
                
                <linearGradient id="dataLayer3" x1="22" y1="44" x2="42" y2="44" gradientUnits="userSpaceOnUse">
                  <stop stopColor="#8b5cf6"/>
                  <stop offset="1" stopColor="#06b6d4"/>
                </linearGradient>
                
                <linearGradient id="connectionLines" x1="0" y1="0" x2="64" y2="64" gradientUnits="userSpaceOnUse">
                  <stop stopColor="#ffffff"/>
                  <stop offset="1" stopColor="#e2e8f0"/>
                </linearGradient>
                
                <radialGradient id="coreGradient" cx="32" cy="32" r="6" gradientUnits="userSpaceOnUse">
                  <stop stopColor="#ffffff" stopOpacity="0.9"/>
                  <stop offset="0.7" stopColor="#06b6d4" stopOpacity="0.8"/>
                  <stop offset="1" stopColor="#3b82f6" stopOpacity="0.6"/>
                </radialGradient>
                
                <linearGradient id="coreStroke" x1="26" y1="26" x2="38" y2="38" gradientUnits="userSpaceOnUse">
                  <stop stopColor="#ffffff"/>
                  <stop offset="1" stopColor="#06b6d4"/>
                </linearGradient>
                
                <linearGradient id="dataFlow" x1="26" y1="32" x2="38" y2="32" gradientUnits="userSpaceOnUse">
                  <stop stopColor="#06b6d4"/>
                  <stop offset="0.5" stopColor="#ffffff"/>
                  <stop offset="1" stopColor="#3b82f6"/>
                </linearGradient>
              </defs>
            </svg>
          </TechIcon>
          <h1 className={styles.heroTitle}>
            What's EpiphanyDB?
          </h1>
          <p className={styles.heroSubtitle}>An open-source, cloud native, horizontally scalable, distributed time-series database.</p>
          <div className={styles.buttons}>
            <Link
              className="button button--primary button--lg"
              to="/docs/intro">
              Getting Started
            </Link>
            <Link
              className="button button--secondary button--lg"
              to="https://github.com/ULis3h/epiphanyDB"
              target="_blank">
              View on GitHub
            </Link>
          </div>
        </div>
      </div>
    </header>
  );
}

function HomepageTerminal() {
  return (
    <section className={styles.terminalSection}>
      <div className="container">
 	      <h2 className={styles.sectionTitle}>
          <span className={styles.colorWhite}>From</span>{' '}
          <span className={styles.colorCyan}>Data</span>{' '}
          <span className={styles.colorBlack}>t</span><span className={styles.colorYellow}>o</span>{' '}
          <span className={styles.colorRed}>Epiphany</span>
        </h2>
        <TerminalAnimation />
      </div>
    </section>
  );
}

function LightDivider() {
  return (
    <div className={styles.lightDivider}>
      <div className={styles.lightLine}></div>
    </div>
  );
}

export default function Home(): React.JSX.Element {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout>
      <HomepageHeader />
      <div style={{position: 'relative', height: 0}}>
        <LightDivider />
      </div>
      <HomepageTerminal />
      <div style={{position: 'relative', height: 0}}>
        <LightDivider />
      </div>
      <main>
	<HomepageFeatures />
      </main>
    </Layout>
  );
}
