import type {ReactNode} from 'react';
import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';

type FeatureItem = {
  title: string;
  Svg: React.ComponentType<React.ComponentProps<'svg'>>;
  description: ReactNode;
};

// 数据库性能图标
const DatabasePerformanceSvg = () => (
  <svg width="200" height="200" viewBox="0 0 200 200" fill="none" xmlns="http://www.w3.org/2000/svg">
    <defs>
      <linearGradient id="dbGradient1" x1="0%" y1="0%" x2="100%" y2="100%">
        <stop offset="0%" stopColor="#3b82f6"/>
        <stop offset="100%" stopColor="#8b5cf6"/>
      </linearGradient>
      <linearGradient id="dbGradient2" x1="0%" y1="0%" x2="100%" y2="100%">
        <stop offset="0%" stopColor="#06b6d4"/>
        <stop offset="100%" stopColor="#3b82f6"/>
      </linearGradient>
    </defs>
    {/* 数据库圆柱体 */}
    <ellipse cx="100" cy="60" rx="60" ry="20" fill="url(#dbGradient1)" opacity="0.8"/>
    <ellipse cx="100" cy="100" rx="60" ry="20" fill="url(#dbGradient1)" opacity="0.6"/>
    <ellipse cx="100" cy="140" rx="60" ry="20" fill="url(#dbGradient1)" opacity="0.4"/>
    <rect x="40" y="60" width="120" height="80" fill="url(#dbGradient2)" opacity="0.3"/>
    {/* 性能指示器 */}
    <path d="M160 40L180 60L160 80" stroke="#06b6d4" strokeWidth="3" fill="none"/>
    <circle cx="170" cy="60" r="8" fill="#06b6d4" opacity="0.8"/>
  </svg>
);

// 云原生架构图标
const CloudNativeSvg = () => (
  <svg width="200" height="200" viewBox="0 0 200 200" fill="none" xmlns="http://www.w3.org/2000/svg">
    <defs>
      <linearGradient id="cloudGradient1" x1="0%" y1="0%" x2="100%" y2="100%">
        <stop offset="0%" stopColor="#06b6d4"/>
        <stop offset="100%" stopColor="#3b82f6"/>
      </linearGradient>
      <radialGradient id="cloudGradient2" cx="50%" cy="50%" r="50%">
        <stop offset="0%" stopColor="#ffffff" stopOpacity="0.8"/>
        <stop offset="100%" stopColor="#3b82f6" stopOpacity="0.4"/>
      </radialGradient>
    </defs>
    {/* 云朵 */}
    <path d="M60 100C60 80 76 64 96 64C104 56 116 52 128 52C148 52 164 68 164 88C172 88 180 96 180 104C180 112 172 120 164 120H68C60 120 54 114 54 106C54 102 56 98 60 100Z" 
          fill="url(#cloudGradient1)" opacity="0.7"/>
    {/* 连接节点 */}
    <circle cx="80" cy="140" r="12" fill="url(#cloudGradient2)" stroke="#06b6d4" strokeWidth="2"/>
    <circle cx="120" cy="140" r="12" fill="url(#cloudGradient2)" stroke="#06b6d4" strokeWidth="2"/>
    <circle cx="160" cy="140" r="12" fill="url(#cloudGradient2)" stroke="#06b6d4" strokeWidth="2"/>
    {/* 连接线 */}
    <path d="M80 140L120 140L160 140" stroke="#3b82f6" strokeWidth="2" opacity="0.6"/>
    <path d="M100 100L100 140M140 100L140 140" stroke="#3b82f6" strokeWidth="2" opacity="0.4"/>
  </svg>
);

// 时序数据图标
const TimeSeriesSvg = () => (
  <svg width="200" height="200" viewBox="0 0 200 200" fill="none" xmlns="http://www.w3.org/2000/svg">
    <defs>
      <linearGradient id="timeGradient1" x1="0%" y1="0%" x2="100%" y2="0%">
        <stop offset="0%" stopColor="#8b5cf6"/>
        <stop offset="50%" stopColor="#3b82f6"/>
        <stop offset="100%" stopColor="#06b6d4"/>
      </linearGradient>
      <linearGradient id="timeGradient2" x1="0%" y1="0%" x2="100%" y2="100%">
        <stop offset="0%" stopColor="#06b6d4"/>
        <stop offset="100%" stopColor="#8b5cf6"/>
      </linearGradient>
    </defs>
    {/* 坐标轴 */}
    <path d="M40 160L40 40M40 160L160 160" stroke="url(#timeGradient1)" strokeWidth="3"/>
    {/* 时序数据曲线 */}
    <path d="M40 140C60 120 80 100 100 80C120 60 140 80 160 60" 
          stroke="url(#timeGradient2)" strokeWidth="4" fill="none" strokeLinecap="round"/>
    {/* 数据点 */}
    <circle cx="60" cy="120" r="4" fill="#06b6d4"/>
    <circle cx="100" cy="80" r="4" fill="#3b82f6"/>
    <circle cx="140" cy="80" r="4" fill="#8b5cf6"/>
    {/* 时间标记 */}
    <rect x="45" y="170" width="8" height="8" fill="#3b82f6" opacity="0.6"/>
    <rect x="95" y="170" width="8" height="8" fill="#3b82f6" opacity="0.6"/>
    <rect x="145" y="170" width="8" height="8" fill="#3b82f6" opacity="0.6"/>
  </svg>
);

const FeatureList: FeatureItem[] = [
  {
    title: 'High Performance',
    Svg: DatabasePerformanceSvg,
    description: (
      <>
        Optimized for time-series data with advanced indexing and compression algorithms.
        Delivers exceptional query performance for both real-time and historical data analysis.
      </>
    ),
  },
  {
    title: 'Cloud Native',
    Svg: CloudNativeSvg,
    description: (
      <>
        Built for modern cloud environments with horizontal scalability and distributed architecture.
        Seamlessly integrates with Kubernetes and supports multi-cloud deployments.
      </>
    ),
  },
  {
    title: 'Time-Series Focused',
    Svg: TimeSeriesSvg,
    description: (
      <>
        Specialized for time-series data workloads with built-in time-based partitioning,
        automatic data retention policies, and optimized storage formats.
      </>
    ),
  },
];

function Feature({title, Svg, description}: FeatureItem) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        <Svg className={styles.featureSvg} role="img" />
      </div>
      <div className="text--center padding-horiz--md">
        <Heading as="h3">{title}</Heading>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures(): ReactNode {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
