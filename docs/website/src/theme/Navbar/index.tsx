import React from 'react';
import OriginalNavbar from '@theme-original/Navbar';
import type {WrapperProps} from '@docusaurus/types';
import type NavbarType from '@theme/Navbar';

type Props = WrapperProps<typeof NavbarType>;

export default function NavbarWrapper(props: Props): React.JSX.Element {
  return (
    <OriginalNavbar {...props} />
  );
}