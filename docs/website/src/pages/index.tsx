import type {ReactNode} from 'react';
import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';
import TerminalAnimation from '@site/src/components/TerminalAnimation';

import styles from './index.module.css';

function HomepageHeader() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <header className={clsx('hero hero--primary', styles.heroBanner)}>
      <div className="container">
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
            to="https://github.com/0x1a0b/epiphanyDB"
            target="_blank">
            View on GitHub
          </Link>
        </div>
      </div>
    </header>
  );
}

function HomepageTerminal() {
  return (
    <section className={styles.terminalSection}>
      <div className="container">
        <h2 className={styles.sectionTitle}>EpiphanyDB</h2>
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

export default function Home(): ReactNode {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout>
      <HomepageHeader />
      <div style={{position: 'relative', height: 0}}>
        <LightDivider />
      </div>
      <HomepageTerminal />
      <main>
	<HomepageFeatures />
      </main>
    </Layout>
  );
}
