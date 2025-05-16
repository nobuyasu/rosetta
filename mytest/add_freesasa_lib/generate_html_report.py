#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
FreeSASA結果をHTMLレポート形式に変換するスクリプト

使用方法:
python generate_html_report.py <results_file> <output_html_file>

例:
python generate_html_report.py sasa_results.txt sasa_report.html
"""

import os
import sys
import json
import datetime
import matplotlib.pyplot as plt
from pathlib import Path

def generate_html_report( results_file, output_file ):
    """
    FreeSASAテスト結果からHTMLレポートを生成する
    
    Parameters:
    results_file (str): SASAテスト結果のテキストファイルパス
    output_file (str): 出力するHTMLファイルパス
    """
    # 結果ファイルの読み込み
    with open( results_file, 'r' ) as f:
        lines = f.readlines()
    
    # パース
    residue_data = []
    total_sasa = None
    
    for line in lines:
        line = line.strip()
        if line.startswith( '#' ):
            if "Total SASA:" in line:
                total_sasa = float( line.split( ":" )[1].strip().split()[0] )
            continue
        
        if line:
            parts = line.split()
            if len( parts ) >= 3:
                residue_data.append({
                    'residue_number': int( parts[0] ),
                    'residue_name': parts[1],
                    'sasa': float( parts[2] )
                })
    
    # グラフの生成（残基ごとのSASA値）
    plt.figure( figsize=( 10, 6 ) )
    residue_numbers = [d['residue_number'] for d in residue_data]
    sasa_values = [d['sasa'] for d in residue_data]
    plt.bar( residue_numbers, sasa_values )
    plt.xlabel( 'Residue Number' )
    plt.ylabel( 'SASA (Å²)' )
    plt.title( 'Per-Residue SASA Values' )
    
    # グラフを一時的に保存
    graph_file = os.path.join( os.path.dirname( output_file ), 'sasa_graph.png' )
    plt.savefig( graph_file )
    
    # HTMLの生成
    html_content = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>FreeSASA Test Results</title>
        <style>
            body {{ font-family: Arial, sans-serif; margin: 20px; }}
            h1, h2 {{ color: #2c3e50; }}
            table {{ border-collapse: collapse; width: 100%; margin-bottom: 20px; }}
            th, td {{ border: 1px solid #ddd; padding: 8px; text-align: left; }}
            th {{ background-color: #f2f2f2; }}
            tr:nth-child(even) {{ background-color: #f9f9f9; }}
            .graph {{ margin: 20px 0; }}
            .summary {{ font-size: 18px; margin: 20px 0; }}
            .timestamp {{ color: #7f8c8d; font-size: 14px; }}
        </style>
    </head>
    <body>
        <h1>FreeSASA Test Results</h1>
        <p class="timestamp">Generated on: {datetime.datetime.now().strftime( '%Y-%m-%d %H:%M:%S' )}</p>
        
        <div class="summary">
            <p><strong>Total SASA:</strong> {total_sasa} Å²</p>
            <p><strong>Total Residues:</strong> {len( residue_data )}</p>
        </div>
        
        <h2>Per-Residue SASA Values</h2>
        <div class="graph">
            <img src="sasa_graph.png" alt="SASA Graph" style="max-width: 100%;">
        </div>
        
        <h2>Detailed Results</h2>
        <table>
            <tr>
                <th>Residue Number</th>
                <th>Residue Name</th>
                <th>SASA (Å²)</th>
            </tr>
    """
    
    # テーブルに残基ごとのデータを追加
    for data in residue_data:
        html_content += f"""
            <tr>
                <td>{data['residue_number']}</td>
                <td>{data['residue_name']}</td>
                <td>{data['sasa']:.2f}</td>
            </tr>
        """
    
    html_content += """
        </table>
    </body>
    </html>
    """
    
    # HTMLファイルの保存
    with open( output_file, 'w' ) as f:
        f.write( html_content )
    
    print( f"HTML report generated: {output_file}" )
    print( f"Graph image saved: {graph_file}" )

def generate_comparison_report( results_lr_file, results_sr_file, output_file ):
    """
    Lee-RichardsとShrake-Rupleyアルゴリズムの結果を比較するHTMLレポートを生成
    
    Parameters:
    results_lr_file (str): Lee-Richardsアルゴリズムの結果ファイル
    results_sr_file (str): Shrake-Rupleyアルゴリズムの結果ファイル
    output_file (str): 出力HTMLファイル
    """
    # 両方のファイルを読み込み
    lr_data = parse_results_file( results_lr_file )
    sr_data = parse_results_file( results_sr_file )
    
    # 差分の計算
    diff_data = []
    for lr_item in lr_data['residues']:
        rsd_num = lr_item['residue_number']
        # 対応するShrake-Rupleyのデータを検索
        sr_item = next(( item for item in sr_data['residues'] if item['residue_number'] == rsd_num ), None )
        
        if sr_item:
            diff = sr_item['sasa'] - lr_item['sasa']
            percent_diff = (diff / lr_item['sasa']) * 100 if lr_item['sasa'] > 0 else 0
            
            diff_data.append({
                'residue_number': rsd_num,
                'residue_name': lr_item['residue_name'],
                'lr_sasa': lr_item['sasa'],
                'sr_sasa': sr_item['sasa'],
                'diff': diff,
                'percent_diff': percent_diff
            })
    
    # 比較グラフの生成
    plt.figure( figsize=( 12, 6 ) )
    residue_numbers = [d['residue_number'] for d in diff_data]
    
    # 差分のプロット
    plt.bar( residue_numbers, [d['diff'] for d in diff_data] )
    plt.axhline( y=0, color='r', linestyle='-' )
    plt.xlabel( 'Residue Number' )
    plt.ylabel( 'SASA Difference (SR - LR) Å²' )
    plt.title( 'SASA Calculation: Shrake-Rupley vs Lee-Richards' )
    
    # グラフを一時的に保存
    graph_file = os.path.join( os.path.dirname( output_file ), 'sasa_comparison_graph.png' )
    plt.savefig( graph_file )
    
    # HTMLの生成
    # HTMLコンテンツの詳細は省略...
    html_content = f"""
    <!DOCTYPE html>
    <html>
    <head>
        <title>FreeSASA Algorithm Comparison</title>
        <style>
            body {{ font-family: Arial, sans-serif; margin: 20px; }}
            h1, h2 {{ color: #2c3e50; }}
            table {{ border-collapse: collapse; width: 100%; margin-bottom: 20px; }}
            th, td {{ border: 1px solid #ddd; padding: 8px; text-align: left; }}
            th {{ background-color: #f2f2f2; }}
            tr:nth-child(even) {{ background-color: #f9f9f9; }}
            .graph {{ margin: 20px 0; }}
            .summary {{ font-size: 18px; margin: 20px 0; }}
            .timestamp {{ color: #7f8c8d; font-size: 14px; }}
            .negative {{ color: red; }}
            .positive {{ color: green; }}
        </style>
    </head>
    <body>
        <h1>FreeSASA Algorithm Comparison</h1>
        <p class="timestamp">Generated on: {datetime.datetime.now().strftime( '%Y-%m-%d %H:%M:%S' )}</p>
        
        <div class="summary">
            <p><strong>Lee-Richards Total SASA:</strong> {lr_data['total_sasa']} Å²</p>
            <p><strong>Shrake-Rupley Total SASA:</strong> {sr_data['total_sasa']} Å²</p>
            <p><strong>Difference:</strong> {sr_data['total_sasa'] - lr_data['total_sasa']:.2f} Å² ({((sr_data['total_sasa'] - lr_data['total_sasa']) / lr_data['total_sasa'] * 100):.2f}%)</p>
        </div>
        
        <h2>Algorithm Comparison</h2>
        <div class="graph">
            <img src="sasa_comparison_graph.png" alt="SASA Comparison Graph" style="max-width: 100%;">
        </div>
        
        <h2>Detailed Comparison</h2>
        <table>
            <tr>
                <th>Residue</th>
                <th>Name</th>
                <th>Lee-Richards (Å²)</th>
                <th>Shrake-Rupley (Å²)</th>
                <th>Difference (Å²)</th>
                <th>Difference (%)</th>
            </tr>
    """
    
    # テーブルに残基ごとの比較データを追加
    for data in diff_data:
        diff_class = "negative" if data['diff'] < 0 else "positive"
        html_content += f"""
            <tr>
                <td>{data['residue_number']}</td>
                <td>{data['residue_name']}</td>
                <td>{data['lr_sasa']:.2f}</td>
                <td>{data['sr_sasa']:.2f}</td>
                <td class="{diff_class}">{data['diff']:.2f}</td>
                <td class="{diff_class}">{data['percent_diff']:.2f}%</td>
            </tr>
        """
    
    html_content += """
        </table>
    </body>
    </html>
    """
    
    # HTMLファイルの保存
    with open( output_file, 'w' ) as f:
        f.write( html_content )
    
    print( f"Comparison HTML report generated: {output_file}" )
    print( f"Comparison graph image saved: {graph_file}" )

def parse_results_file( results_file ):
    """結果ファイルを解析してデータ構造を返す"""
    with open( results_file, 'r' ) as f:
        lines = f.readlines()
    
    residue_data = []
    total_sasa = None
    
    for line in lines:
        line = line.strip()
        if line.startswith( '#' ):
            if "Total SASA:" in line:
                total_sasa = float( line.split( ":" )[1].strip().split()[0] )
            continue
        
        if line:
            parts = line.split()
            if len( parts ) >= 3:
                residue_data.append({
                    'residue_number': int( parts[0] ),
                    'residue_name': parts[1],
                    'sasa': float( parts[2] )
                })
    
    return {
        'residues': residue_data,
        'total_sasa': total_sasa
    }

if __name__ == "__main__":
    if len( sys.argv ) < 3:
        print( "Usage: python generate_html_report.py <results_file> <output_html_file>" )
        print( "  or" )
        print( "Usage: python generate_html_report.py compare <lr_results_file> <sr_results_file> <output_html_file>" )
        sys.exit( 1 )
    
    if sys.argv[1] == 'compare' and len( sys.argv ) >= 5:
        generate_comparison_report( sys.argv[2], sys.argv[3], sys.argv[4] )
    else:
        generate_html_report( sys.argv[1], sys.argv[2] )