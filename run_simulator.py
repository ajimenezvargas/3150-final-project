#!/usr/bin/env python3
"""
BGP Simulator Python Wrapper
Provides a clean Python interface to the C++ BGP simulator
"""

import subprocess
import sys
import os
import argparse

def run_simulator(caida_file, announcements_file, rov_asns_file=None, output_file="ribs.csv"):
    """
    Run the BGP simulator with the given input files
    
    Args:
        caida_file: Path to CAIDA AS relationships file
        announcements_file: Path to announcements CSV (asn,prefix,rov_invalid)
        rov_asns_file: Optional path to ROV ASNs CSV  
        output_file: Path to output CSV file (default: ribs.csv)
    
    Returns:
        int: Exit code (0 for success)
    """
    
    # Check if simulator binary exists
    simulator_path = "./simulator"
    if not os.path.exists(simulator_path):
        print("Error: simulator binary not found. Run 'make' first.", file=sys.stderr)
        return 1
    
    # Build command
    cmd = [
        simulator_path,
        "--caida", caida_file,
        "--announcements", announcements_file,
        "--output", output_file
    ]
    
    if rov_asns_file:
        cmd.extend(["--rov-asns", rov_asns_file])
    
    # Run simulator
    try:
        result = subprocess.run(cmd, check=False)
        return result.returncode
    except Exception as e:
        print(f"Error running simulator: {e}", file=sys.stderr)
        return 1

def main():
    parser = argparse.ArgumentParser(
        description="BGP Simulator - Cloudflare Network Optimization",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python3 run_simulator.py --caida data/relationships.txt \\
                          --announcements data/announcements.csv \\
                          --output ribs.csv

  python3 run_simulator.py --caida data/relationships.txt \\
                          --announcements data/announcements.csv \\
                          --rov-asns data/rov_asns.csv \\
                          --output ribs.csv
        """
    )
    
    parser.add_argument('--caida', required=True,
                       help='Path to CAIDA AS relationships file')
    parser.add_argument('--announcements', required=True,
                       help='Path to announcements CSV file (asn,prefix,rov_invalid)')
    parser.add_argument('--rov-asns',
                       help='Path to ROV ASNs CSV file (optional)')
    parser.add_argument('--output', default='ribs.csv',
                       help='Path to output CSV file (default: ribs.csv)')
    
    args = parser.parse_args()
    
    # Validate input files exist
    if not os.path.exists(args.caida):
        print(f"Error: CAIDA file not found: {args.caida}", file=sys.stderr)
        return 1
    
    if not os.path.exists(args.announcements):
        print(f"Error: Announcements file not found: {args.announcements}", file=sys.stderr)
        return 1
    
    if args.rov_asns and not os.path.exists(args.rov_asns):
        print(f"Error: ROV ASNs file not found: {args.rov_asns}", file=sys.stderr)
        return 1
    
    # Run simulator
    return run_simulator(
        args.caida,
        args.announcements,
        args.rov_asns,
        args.output
    )

if __name__ == "__main__":
    sys.exit(main())