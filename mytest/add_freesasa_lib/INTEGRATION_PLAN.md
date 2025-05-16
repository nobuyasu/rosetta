# FreeSASA - Rosetta 統合計画書

## 1. プロジェクト概要

### 1.1 目的

RosettaフレームワークにFreeSASA（Fast, Exact Solvent Accessible Surface Area）ライブラリを統合し、分子表面積計算機能を追加する。FreeSASAは軽量でポータブルなSASA計算ライブラリであり、Rosettaのアプリケーションやプロトコルで直接利用可能にする。

### 1.2 背景

タンパク質表面の特性（特に溶媒接触表面積）は、タンパク質の安定性や相互作用性などを理解する上で重要な指標である。現在、Rosettaは独自のSASA計算方法を持っているが、FreeSASAのような専用ライブラリを統合することで、以下の利点が期待できる：

- 計算精度と速度の向上
- 複数のアルゴリズム（Lee-Richards/Shrake-Rupley）の選択肢
- 標準化されたSASA計算へのアクセス
- タンパク質設計や解析パイプラインでの活用

### 1.3 プロジェクト成果物

1. Rosettaにビルドインされた状態のFreeSASA統合
2. Rosettaのコード規約に準拠したインターフェースクラス
3. 単体テストとサンプルアプリケーション
4. 使用方法とAPIドキュメント

## 2. プロジェクト計画

### 2.1 開発ブランチ

```bash
# 現在のブランチから新しいブランチを作成
git checkout -b add_freesasa_lib
```

名前: `add_freesasa_lib`  
説明: FreeSASA統合のための開発ブランチ

### 2.2 統合フェーズとタスク

#### フェーズ1: 準備作業とFreeSASAのRosettaへの組み込み

1. **FreeSASAソースコードの配置**
   ```bash
   # FreeSASAのソースコードをRosettaの外部ライブラリディレクトリにコピー
   mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa
   cp -r /Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/freesasa/* /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/
   
   # ライブラリディレクトリの作成
   mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/lib
   cp /Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/lib/libfreesasa.a /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/lib/
   ```

2. **FreeSASA設定ファイルの作成**
   ```bash
   # 外部ライブラリ設定ファイルを作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/external/freesasa.external.settings
   ```
   
   設定ファイルの内容:
   ```python
   # FreeSASA external library settings
   import os
   
   def settings(env):
       env.Append(CPPDEFINES=["USE_FREESASA"])
       env.Append(CPPPATH=[os.path.join(env["EXTERNAL_TOOLS_PATH"],"freesasa/src")])
       env.Append(LIBPATH=[os.path.join(env["EXTERNAL_TOOLS_PATH"],"freesasa/lib")])
       env.Append(LIBS=["freesasa"])
       return True
   ```

3. **SConscript.externalの更新**
   
   ```bash
   # SConscript.externalファイルを編集
   vim /Users/nobuyasu/dev_2025/rosetta/source/external/SConscript.external
   ```
   
   追加する内容:
   ```python
   # FreeSASA
   if env.get('use_freesasa', False):
       freesasa_success = env.get('FREESASA_SUCCESS', False)
       if freesasa_success or SConscript('freesasa.external.settings', exports=['env']):
           env['FREESASA_SUCCESS'] = True
   ```

4. **CMake設定の更新**
   
   ```bash
   # CMakeの設定ファイルを編集
   vim /Users/nobuyasu/dev_2025/rosetta/source/cmake/build_settings/all_lib_settings.cmake
   ```
   
   追加する内容:
   ```cmake
   # FreeSASA Library
   option(USE_FREESASA "Enable FreeSASA support" ON)
   if(USE_FREESASA)
     message(STATUS "Building with FreeSASA support")
     add_definitions(-DUSE_FREESASA)
     include_directories(${CMAKE_SOURCE_DIR}/external/freesasa/src)
     link_directories(${CMAKE_SOURCE_DIR}/external/freesasa/lib)
   endif()
   ```

#### フェーズ2: インターフェースクラスの設計と実装

1. **コアディレクトリ構造の作成**
   
   ```bash
   # インターフェースクラス用のディレクトリを作成
   mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa
   ```

2. **ヘッダーファイルの作成**
   
   ```bash
   # FreeSASAインターフェースヘッダを作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa/FreeSASA.hh
   ```
   
   詳細は後述の「インターフェース仕様」セクションを参照。

3. **実装ファイルの作成**
   
   ```bash
   # FreeSASAインターフェース実装を作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa/FreeSASA.cc
   ```

4. **src.settings ファイルの更新**
   
   ```bash
   # src.settingsファイルを編集 (core.3.src.settingsが最適)
   vim /Users/nobuyasu/dev_2025/rosetta/source/src/core.3.src.settings
   ```
   
   追加する内容:
   ```
   core/scoring/sasa
   ```

#### フェーズ3: テストとドキュメントの作成

1. **単体テストの作成**
   
   ```bash
   # テストディレクトリの作成
   mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/test/core/scoring/sasa
   
   # テストファイルの作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/test/core/scoring/sasa/FreeSASATest.cxxtest.hh
   ```

2. **サンプルアプリケーションの作成** (オプション)
   
   ```bash
   # アプリケーションディレクトリの作成
   mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/src/apps/public/analysis
   
   # アプリケーションファイルの作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/src/apps/public/analysis/CalculateSASA.cc
   ```

3. **ドキュメントの作成**
   
   ```bash
   # READMEファイルの作成
   touch /Users/nobuyasu/dev_2025/rosetta/source/src/core/scoring/sasa/README.md
   ```

#### フェーズ4: Xcodeプロジェクト設定

1. **Xcodeプロジェクト生成**

   ```bash
   # Xcodeプロジェクト生成ディレクトリに移動
   cd /Users/nobuyasu/dev_2025/rosetta/source/xcode
   
   # FreeSASA対応のXcodeプロジェクト生成
   python make_project.py --extras=freesasa
   ```

2. **Xcodeプロジェクト設定の微調整**

   - Xcodeでプロジェクトを開く
   - ヘッダ検索パスに`/Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/src`を追加
   - ライブラリ検索パスに`/Users/nobuyasu/dev_2025/rosetta/source/external/freesasa/lib`を追加
   - `libfreesasa.a`をプロジェクトにリンク

3. **プロジェクト構成の保存**

   ```bash
   # プロジェクト構成をカスタムファイルとして保存（オプション）
   cp -r /Users/nobuyasu/dev_2025/rosetta/source/xcode/Rosetta.xcodeproj /Users/nobuyasu/dev_2025/rosetta/source/xcode/Rosetta_FreeSASA.xcodeproj
   ```

#### フェーズ5: ビルドとテスト

1. **コマンドラインでのビルド**

   ```bash
   # SCons を使用したビルド
   cd /Users/nobuyasu/dev_2025/rosetta/source
   ./scons.py -j4 mode=release extras=freesasa bin
   
   # CMake を使用したビルド
   cd /Users/nobuyasu/dev_2025/rosetta/source/cmake
   mkdir -p build_freesasa && cd build_freesasa
   cmake .. -DCMAKE_BUILD_TYPE=Release -DUSE_FREESASA=ON
   make -j4
   ```

2. **Xcodeでのビルド**

   - Xcodeプロジェクトを開く
   - ターゲットを「core」や特定のアプリケーションに設定
   - ビルド設定でリリースモードを選択
   - ビルドを実行

3. **テストの実行**

   ```bash
   # SCons を使用したテスト
   cd /Users/nobuyasu/dev_2025/rosetta/source
   ./scons.py -j4 mode=release extras=freesasa cat=core.scoring.sasa
   
   # CMake を使用したテスト
   cd /Users/nobuyasu/dev_2025/rosetta/source/cmake/build_freesasa
   ctest -R FreeSASATest
   ```

## 3. インターフェース仕様

### 3.1 クラス図

```
+------------------------------------------+
| FreeSASA                                 |
+------------------------------------------+
| - probe_radius_: Real                    |
| - algorithm_: int                        |
| - resolution_: int                       |
+------------------------------------------+
| + FreeSASA(probe_radius: Real = 1.4)     |
| + ~FreeSASA()                            |
| + calculateAtomSASA(pose: Pose): vector1 |
| + calculateResidueSASA(pose: Pose): map  |
| + setAlgorithm(alg: int, res: int)       |
| + setProbeRadius(radius: Real)           |
| - createStructure(pose: Pose): void*     |
+------------------------------------------+
```

### 3.2 ヘッダーファイル (FreeSASA.hh)

```cpp
// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the
// (c) Rosetta Commons. For more information, see http://www.rosettacommons.org.
// (c) Questions about this can be addressed to University of Washington UW TechTransfer,
// (c) email: license@u.washington.edu.

/// @file core/scoring/sasa/FreeSASA.hh
/// @brief FreeSASA library integration for SASA calculations
/// @author Nobuyasu Koga (nobuyasu@u.washington.edu)

#ifndef INCLUDED_core_scoring_sasa_FreeSASA_hh
#define INCLUDED_core_scoring_sasa_FreeSASA_hh

// Package headers
#include <core/types.hh>
#include <core/pose/Pose.fwd.hh>
#include <core/id/AtomID.fwd.hh>
#include <utility/vector1.hh>

// C++ headers
#include <map>
#include <string>

// Forward declarations for FreeSASA
struct freesasa_structure;
struct freesasa_result;
struct freesasa_parameters;

namespace core {
namespace scoring {
namespace sasa {

/// @brief Interface to FreeSASA library for SASA calculations
///
/// This class provides an interface to the FreeSASA library 
/// (https://freesasa.github.io/) for calculating solvent 
/// accessible surface area (SASA) of proteins.
class FreeSASA {
public:
  /// @brief Constructor with default parameters
  /// @param probe_radius The probe radius in Angstroms ( default 1.4 )
  FreeSASA( core::Real probe_radius = 1.4 );
  
  /// @brief Destructor
  ~FreeSASA();
  
  /// @brief Calculate SASA for all atoms in a pose
  /// @param pose The pose to calculate SASA for
  /// @return A map of atom IDs to SASA values
  std::map< core::id::AtomID, core::Real > 
  calculateAtomSASA( core::pose::Pose const & pose );
  
  /// @brief Calculate SASA for all residues in a pose
  /// @param pose The pose to calculate SASA for
  /// @return A map of residue indices to SASA values
  std::map< core::Size, core::Real > 
  calculateResidueSASA( core::pose::Pose const & pose );
  
  /// @brief Set the algorithm for SASA calculation
  /// @param algorithm 0 for Lee-Richards, 1 for Shrake-Rupley
  /// @param resolution Number of slices for Lee-Richards or test points for Shrake-Rupley
  void setAlgorithm( int algorithm, int resolution = 20 );
  
  /// @brief Set the probe radius
  /// @param radius The probe radius in Angstroms
  void setProbeRadius( core::Real radius );
  
private:
  /// @brief Create a FreeSASA structure from a Rosetta pose
  /// @param pose The pose to convert
  /// @param skip_hydrogens Whether to skip hydrogen atoms
  /// @return A pointer to the created FreeSASA structure ( caller must free )
  freesasa_structure* createStructureFromPose(
    core::pose::Pose const & pose,
    bool skip_hydrogens = true );
  
  /// @brief Initialize default parameters
  void initParameters();
  
  /// @brief Determine element from atom name
  /// @param atom_name The atom name
  /// @return The element symbol
  std::string getAtomElement( std::string const & atom_name );
  
  // Class members
  core::Real probe_radius_;
  int algorithm_;      // 0 = Lee-Richards, 1 = Shrake-Rupley
  int resolution_;     // Slices or test points
  bool initialized_;
};

} // namespace sasa
} // namespace scoring
} // namespace core

#endif // INCLUDED_core_scoring_sasa_FreeSASA_hh
```

### 3.3 実装の基本構造 (FreeSASA.cc)

```cpp
// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// (c) Copyright Rosetta Commons Member Institutions.
// (c) This file is part of the Rosetta software suite and is made available under license.
// (c) The Rosetta software is developed by the contributing members of the
// (c) Rosetta Commons. For more information, see http://www.rosettacommons.org.
// (c) Questions about this can be addressed to University of Washington UW TechTransfer,
// (c) email: license@u.washington.edu.

/// @file core/scoring/sasa/FreeSASA.cc
/// @brief Implementation of FreeSASA interface
/// @author Nobuyasu Koga (nobuyasu@u.washington.edu)

// Unit headers
#include <core/scoring/sasa/FreeSASA.hh>

// Package headers
#include <core/pose/Pose.hh>
#include <core/conformation/Residue.hh>
#include <core/id/AtomID.hh>
#include <basic/Tracer.hh>

// External headers
#ifdef USE_FREESASA
#include <freesasa.h>
#endif

// C++ headers
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>

namespace core {
namespace scoring {
namespace sasa {

static basic::Tracer TR("core.scoring.sasa.FreeSASA");

// Constructor implementation
FreeSASA::FreeSASA(core::Real probe_radius) :
  probe_radius_(probe_radius),
  algorithm_(0),        // Lee-Richards by default
  resolution_(20),
  initialized_(false)
{
#ifdef USE_FREESASA
  // Set FreeSASA to silent mode
  freesasa_set_verbosity(FREESASA_V_SILENT);
  initialized_ = true;
#else
  TR.Warning << "FreeSASA support not compiled in. SASA calculations will fail." << std::endl;
  initialized_ = false;
#endif
}

// Destructor implementation
FreeSASA::~FreeSASA() {
  // No manual cleanup needed
}

// Set algorithm type
void
FreeSASA::setAlgorithm(int algorithm, int resolution) {
  algorithm_ = algorithm;
  resolution_ = resolution;
}

// Set probe radius
void
FreeSASA::setProbeRadius(core::Real radius) {
  probe_radius_ = radius;
}

// Calculate SASA for all atoms in a pose
std::map<core::id::AtomID, core::Real> 
FreeSASA::calculateAtomSASA(core::pose::Pose const & pose) {
  std::map<core::id::AtomID, core::Real> atom_sasa;
  
#ifdef USE_FREESASA
  // Create FreeSASA structure from pose
  freesasa_structure* structure = createStructureFromPose(pose);
  if (!structure) {
    TR.Error << "Failed to create FreeSASA structure from pose." << std::endl;
    return atom_sasa;
  }
  
  // Set calculation parameters
  freesasa_parameters parameters;
  if (algorithm_ == 0) {
    parameters = freesasa_parameters_lee_richards();
  } else {
    parameters = freesasa_parameters_shrake_rupley();
  }
  parameters.probe_radius = probe_radius_;
  parameters.n_slices = resolution_;
  parameters.n_points = resolution_;
  
  // Calculate SASA
  freesasa_result* result = freesasa_calc_structure(structure, &parameters);
  if (!result) {
    TR.Error << "FreeSASA calculation failed." << std::endl;
    freesasa_structure_free(structure);
    return atom_sasa;
  }
  
  // Map results to atom IDs
  const double* values = freesasa_result_values(result);
  int n_atoms = freesasa_structure_n(structure);
  
  int atom_count = 0;
  for (core::Size ires = 1; ires <= pose.size(); ++ires) {
    core::conformation::Residue const & rsd = pose.residue(ires);
    for (core::Size iatom = 1; iatom <= rsd.natoms(); ++iatom) {
      if (atom_count < n_atoms) {
        core::id::AtomID atom_id(iatom, ires);
        atom_sasa[atom_id] = values[atom_count];
        atom_count++;
      }
    }
  }
  
  // Cleanup
  freesasa_result_free(result);
  freesasa_structure_free(structure);
#else
  TR.Error << "FreeSASA support not compiled in. Cannot calculate SASA." << std::endl;
#endif
  
  return atom_sasa;
}

// Calculate SASA for all residues in a pose
std::map<core::Size, core::Real> 
FreeSASA::calculateResidueSASA(core::pose::Pose const & pose) {
  std::map<core::Size, core::Real> residue_sasa;
  
#ifdef USE_FREESASA
  // Get atom-level SASA
  std::map<core::id::AtomID, core::Real> atom_sasa = calculateAtomSASA(pose);
  
  // Sum up SASA by residue
  for (auto const & atom_pair : atom_sasa) {
    core::id::AtomID const & atom_id = atom_pair.first;
    core::Real const sasa_value = atom_pair.second;
    
    core::Size const residue_index = atom_id.rsd();
    residue_sasa[residue_index] += sasa_value;
  }
#else
  TR.Error << "FreeSASA support not compiled in. Cannot calculate SASA." << std::endl;
#endif
  
  return residue_sasa;
}

// Create FreeSASA structure from pose
freesasa_structure* 
FreeSASA::createStructureFromPose(core::pose::Pose const & pose, bool skip_hydrogens) {
#ifdef USE_FREESASA
  // Count atoms first (excluding H if requested)
  int n_atoms = 0;
  for (core::Size ires = 1; ires <= pose.size(); ++ires) {
    core::conformation::Residue const & rsd = pose.residue(ires);
    for (core::Size iatom = 1; iatom <= rsd.natoms(); ++iatom) {
      std::string const & atom_name = rsd.atom_name(iatom);
      if (!skip_hydrogens || (atom_name.size() > 0 && atom_name[0] != 'H')) {
        n_atoms++;
      }
    }
  }
  
  // Allocate memory for atom data
  double* xyzr = new double[4 * n_atoms];
  char** atoms = new char*[n_atoms];
  char** residues = new char*[n_atoms];
  
  // Fill atom data
  int atom_count = 0;
  for (core::Size ires = 1; ires <= pose.size(); ++ires) {
    core::conformation::Residue const & rsd = pose.residue(ires);
    std::string const & res_name = rsd.name3();
    
    for (core::Size iatom = 1; iatom <= rsd.natoms(); ++iatom) {
      std::string const & atom_name = rsd.atom_name(iatom);
      
      // Skip hydrogens if requested
      if (skip_hydrogens && atom_name.size() > 0 && atom_name[0] == 'H') {
        continue;
      }
      
      // Get coordinates
      core::Vector const & xyz = rsd.xyz(iatom);
      xyzr[4*atom_count] = xyz.x();
      xyzr[4*atom_count+1] = xyz.y();
      xyzr[4*atom_count+2] = xyz.z();
      
      // Get radius (using element-based defaults)
      std::string element = getAtomElement(atom_name);
      xyzr[4*atom_count+3] = freesasa_classifier_radius(freesasa_default_classifier(), 
                                                        res_name.c_str(), atom_name.c_str());
      
      // Store atom and residue names
      atoms[atom_count] = strdup(atom_name.c_str());
      residues[atom_count] = strdup(res_name.c_str());
      
      atom_count++;
    }
  }
  
  // Create FreeSASA structure
  freesasa_structure* structure = freesasa_structure_from_arrays(n_atoms, xyzr, 
                                                               (const char**)atoms, 
                                                               (const char**)residues);
  
  // Cleanup temporary arrays
  delete[] xyzr;
  for (int i = 0; i < n_atoms; i++) {
    free(atoms[i]);
    free(residues[i]);
  }
  delete[] atoms;
  delete[] residues;
  
  return structure;
#else
  return nullptr;
#endif
}

// Determine element from atom name
std::string
FreeSASA::getAtomElement(std::string const & atom_name) {
  if (atom_name.empty()) return "C";
  
  // Extract first character (possible element)
  char first_char = atom_name[0];
  
  // Common elements in proteins
  if (first_char == 'C') return "C";
  if (first_char == 'N') return "N";
  if (first_char == 'O') return "O";
  if (first_char == 'S') return "S";
  if (first_char == 'H') return "H";
  if (first_char == 'P') return "P";
  
  // Default to carbon
  return "C";
}

} // namespace sasa
} // namespace scoring
} // namespace core
```

## 4. 設計原則と考慮事項

### 4.1 コードスタイルと命名規則

- Rosettaコーディング規約に厳密に従う
- クラス名は大文字始まりのCamelCase
- メソッド名は小文字始まりのcamelCase
- 変数名はスネークケース
- マクロと定数は大文字のSNAKE_CASE

### 4.2 エラー処理

- 全ての関数でRosettaスタイルのエラー処理を実装
- FreeSASAが利用できない場合は、明確なエラーメッセージを表示
- 異常なパラメータや計算失敗に対して適切に対応

### 4.3 メモリ管理

- Rosettaのメモリ管理パターンに従う
- FreeSASA構造体の適切な解放を保証
- メモリリークを防止するためのRAIIパターンの使用

### 4.4 パフォーマンス

- 大きなタンパク質構造に対する効率的な計算
- 必要に応じた計算結果のキャッシング
- アルゴリズム選択によるパフォーマンスとのトレードオフ

### 4.5 テスト戦略

- 小さなテスト用タンパク質でのSASA計算精度検証
- Rosetta内蔵SASA計算との結果比較テスト
- エッジケース（小さな/大きな構造、特異なトポロジー）のテスト

## 5. 引き継ぎ情報

### 5.1 主要ファイル

- **FreeSASAソース**: `/Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/freesasa/`
- **FreeSASAヘッダ**: `/Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/include/freesasa.h`
- **ライブラリファイル**: `/Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/lib/libfreesasa.a`
- **計画書**: `/Users/nobuyasu/dev_2025/rosetta/mytest/exposed_hydrophobics/INTEGRATION_PLAN.md`

### 5.1.0 作業ディレクトリとテストコード

- **作業ディレクトリ**: 実装中に生成される一時ファイルやビルド結果は `/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/` に保存する
- **テストコード作成場所**: `/Users/nobuyasu/dev_2025/rosetta/source/src/apps/pilot/nobuyasu/` にテストコードを作成する
  ```bash
  # テスト用ディレクトリを作成
  mkdir -p /Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib
  mkdir -p /Users/nobuyasu/dev_2025/rosetta/source/src/apps/pilot/nobuyasu
  
  # テストコードの例
  touch /Users/nobuyasu/dev_2025/rosetta/source/src/apps/pilot/nobuyasu/test_freesasa.cc
  ```

### 5.1.0.1 テストコードの例

以下は `/Users/nobuyasu/dev_2025/rosetta/source/src/apps/pilot/nobuyasu/test_freesasa.cc` に実装するテストコードの例です：

```cpp
// -*- mode:c++;tab-width:2;indent-tabs-mode:t;show-trailing-whitespace:t;rm-trailing-spaces:t -*-
// vi: set ts=2 noet:
//
// This file is part of the Rosetta software suite and is made available under license.
// The Rosetta software is developed by the contributing members of the Rosetta Commons consortium.
// (c) For more information, see http://www.rosettacommons.org.

/// @file /src/apps/pilot/nobuyasu/test_freesasa.cc
/// @brief Test application for FreeSASA integration
/// @author Nobuyasu Koga (nkoga@protein.osaka-u.ac.jp)

// Rosetta headers
#include <core/init.hh>
#include <core/pose/Pose.hh>
#include <core/scoring/sasa/FreeSASA.hh>
#include <core/import_pose/import_pose.hh>
#include <basic/options/option.hh>
#include <basic/options/keys/in.OptionKeys.gen.hh>
#include <basic/Tracer.hh>

// C++ headers
#include <iostream>
#include <iomanip>
#include <fstream>

static basic::Tracer TR("test_freesasa");

using namespace core;

int main(int argc, char *argv[]) {
  try {
    // Initialize Rosetta
    init(argc, argv);
    
    using namespace basic::options;
    using namespace basic::options::OptionKeys;
    
    // Check if input PDB file was provided
    if (!option[in::file::s].user()) {
      TR.Error << "No input PDB file provided. Use -in:file:s to specify." << std::endl;
      return 1;
    }
    
    // Load pose from PDB file
    pose::Pose pose;
    import_pose::pose_from_file(pose, option[in::file::s]()[1]);
    TR.Info << "Loaded pose with " << pose.size() << " residues." << std::endl;
    
    // Create FreeSASA calculator with default parameters
    scoring::sasa::FreeSASA sasa_calc;
    
    // Calculate per-residue SASA
    std::map<Size, Real> residue_sasa = sasa_calc.calculateResidueSASA(pose);
    
    // Calculate per-atom SASA
    std::map<id::AtomID, Real> atom_sasa = sasa_calc.calculateAtomSASA(pose);
    
    // Output results to file
    std::ofstream out_file("/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/sasa_results.txt");
    
    // Per-residue SASA
    out_file << "# Per-residue SASA values:" << std::endl;
    out_file << "#   Residue   SASA (Å²)" << std::endl;
    for (Size i = 1; i <= pose.size(); ++i) {
      std::string res_name = pose.residue(i).name3();
      out_file << std::setw(8) << i << std::setw(5) << res_name 
               << std::setw(12) << std::fixed << std::setprecision(2) << residue_sasa[i] << std::endl;
    }
    
    // Print atom count and total SASA
    TR.Info << "Calculated SASA for " << atom_sasa.size() << " atoms." << std::endl;
    
    // Calculate total SASA
    Real total_sasa = 0.0;
    for (auto const & it : residue_sasa) {
      total_sasa += it.second;
    }
    
    TR.Info << "Total SASA: " << total_sasa << " Å²" << std::endl;
    out_file << "# Total SASA: " << total_sasa << " Å²" << std::endl;
    
    out_file.close();
    
    // Compare with Lee-Richards algorithm at different resolutions
    TR.Info << "Testing Lee-Richards algorithm with different resolutions:" << std::endl;
    for (int res = 10; res <= 40; res += 10) {
      scoring::sasa::FreeSASA sasa_lr(1.4);
      sasa_lr.setAlgorithm(0, res); // 0 = Lee-Richards
      std::map<Size, Real> rsd_sasa_lr = sasa_lr.calculateResidueSASA(pose);
      
      // Calculate total
      Real total = 0.0;
      for (auto const & it : rsd_sasa_lr) total += it.second;
      
      TR.Info << "  Resolution " << res << ": " << total << " Å²" << std::endl;
    }
    
    // Compare with Shrake-Rupley algorithm
    TR.Info << "Testing Shrake-Rupley algorithm:" << std::endl;
    scoring::sasa::FreeSASA sasa_sr(1.4);
    sasa_sr.setAlgorithm(1, 100); // 1 = Shrake-Rupley
    std::map<Size, Real> rsd_sasa_sr = sasa_sr.calculateResidueSASA(pose);
    
    // Calculate total
    Real total_sr = 0.0;
    for (auto const & it : rsd_sasa_sr) total_sr += it.second;
    
    TR.Info << "  Shrake-Rupley: " << total_sr << " Å²" << std::endl;
    TR.Info << "  Difference from Lee-Richards: " << (total_sr - total_sasa) << " Å²" << std::endl;
    
    return 0;
  } catch (utility::excn::Exception const & e) {
    std::cerr << "Exception caught: " << e.msg() << std::endl;
    return 1;
  } catch (std::exception const & e) {
    std::cerr << "STL exception caught: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown exception caught" << std::endl;
    return 1;
  }
}
```

このコードは、FreeSASA インターフェースを使用して:
1. 指定された PDB ファイルの SASA を計算
2. 結果を `/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/sasa_results.txt` に保存
3. Lee-Richards および Shrake-Rupley アルゴリズム間の比較を行います

### 5.1.0.2 テストコードのビルド手順

テストコードをビルドする際は、以下の手順に従います：

1. 新しいファイルを追加した場合、プロジェクトファイルを再生成する
   ```bash
   cd /Users/nobuyasu/dev_2025/rosetta/source/cmake
   ./make_projects.py all
   ```

2. Xcodeプロジェクトも更新する
   ```bash
   cd /Users/nobuyasu/dev_2025/rosetta/source/xcode
   python make_project.py --extras=freesasa
   ```

3. CMakeでビルドを実行
   ```bash
   cd /Users/nobuyasu/dev_2025/rosetta/source/cmake/build_release
   ninja test_freesasa
   ```

4. テストコードを実行
   ```bash
   cd /Users/nobuyasu/dev_2025/rosetta/source/cmake/build_release
   ./bin/test_freesasa -in:file:s /path/to/pdb/file.pdb
   ```

### 5.1.0.3 Xcodeでの開発

新しいソースコードを追加した後は、必ずXcodeプロジェクトも更新します：

1. Xcodeプロジェクト生成
   ```bash
   cd /Users/nobuyasu/dev_2025/rosetta/source/xcode
   python make_project.py --extras=freesasa
   ```

2. Xcodeでプロジェクトを開く
   ```bash
   open /Users/nobuyasu/dev_2025/rosetta/source/xcode/Rosetta.xcodeproj
   ```

3. Xcodeでのビルド設定確認
   - スキーム: `test_freesasa`を選択
   - ビルド設定: `Release`を選択
   - 引数を追加: Edit Scheme > Run > Arguments > + > `-in:file:s path/to/pdb/file.pdb`

4. ビルドとデバッグ
   - 必要に応じてブレークポイントを設定
   - `CMD+B`でビルド
   - `CMD+R`で実行

### 5.1.1 統合後の最終的なファイル構成

```
/Users/nobuyasu/dev_2025/rosetta/
  ├── source/
  │   ├── external/
  │   │   ├── freesasa/                          # FreeSASA ライブラリのソース
  │   │   │   ├── src/                           # FreeSASA のソースコード
  │   │   │   ├── lib/                           # コンパイル済みライブラリファイル
  │   │   │   └── include/                       # FreeSASA のヘッダーファイル
  │   │   │
  │   │   ├── freesasa.external.settings         # SCons 用のFreeSASA設定ファイル
  │   │   └── SConscript.external                # FreeSASA を含むように修正
  │   │
  │   ├── cmake/
  │   │   └── build_settings/
  │   │       └── all_lib_settings.cmake         # FreeSASA のためのCMake設定を追加
  │   │
  │   └── src/
  │       ├── core/
  │       │   └── scoring/
  │       │       └── sasa/                      # SASA計算のためのコアクラス
  │       │           ├── FreeSASA.hh            # FreeSASA インターフェースのヘッダー
  │       │           ├── FreeSASA.cc            # FreeSASA インターフェースの実装
  │       │           └── FreeSASA.fwd.hh        # 前方宣言
  │       │
  │       └── core.2.src.settings                # FreeSASAクラスをビルドシステムに追加
  │
  ├── tests/
  │   └── core/
  │       └── scoring/
  │           └── sasa/
  │               └── FreeSASATest.cxxtest.hh   # FreeSASA 単体テスト
  │
  └── documentation/
      └── freesasa_integration.md               # FreeSASA 統合についてのドキュメント
```

### 5.2 開発ブランチ

新規ブランチ名: `add_freesasa_lib`  
基準ブランチ: `main` または `master`

### 5.3 コミット計画

1. **初期セットアップ**:
   - FreeSASAライブラリの追加
   - ビルドスクリプトの更新

2. **インターフェース実装**:
   - コアクラスの実装
   - 基本機能のテスト

3. **テストとドキュメント**:
   - 単体テストの追加
   - ドキュメントの整備

4. **サンプルアプリケーション**:
   - 簡単なSASA計算アプリの提供

### 5.4 テスト手順

1. 単体テストの実行
2. 簡単なPDBファイル（例: 2GB1.pdb）でのSASA計算テスト
3. FreeSASAの直接実行結果との比較検証

## 6. 参考リソース

### 6.1 FreeSASA関連

- **公式サイト**: https://freesasa.github.io/
- **API ドキュメント**: https://freesasa.github.io/doxygen/
- **GitHub リポジトリ**: https://github.com/mittinatten/freesasa

### 6.2 Rosetta関連

- **Rosetta デベロッパーガイド**: https://www.rosettacommons.org/docs/latest/development_documentation/
- **Xcode プロジェクト生成**: https://www.rosettacommons.org/docs/latest/development_documentation/IDEs/xcode
- **外部ライブラリ統合**: https://www.rosettacommons.org/docs/latest/development_documentation/build_system/external_libraries

## 7. ライセンス情報

- **FreeSASA**: MIT ライセンス
- **Rosetta**: Rosetta Commons ライセンス
- **統合コード**: Rosetta Commons ライセンスが適用される予定

## 8. 追加考慮事項

### 8.1 テスト方法

FreeSASA 統合のテストは、以下の方法で行います：

1. **単体テスト (CxxTest)**
   FreeSASA インターフェースの主要な機能をテストするための単体テストを実装します。
   ```cpp
   // /Users/nobuyasu/dev_2025/rosetta/source/test/core/scoring/sasa/FreeSASATest.cxxtest.hh
   // FreeSASA クラスの単体テスト実装
   ```

2. **パイロットアプリケーション**
   FreeSASA の機能をテストするための独自のパイロットアプリケーションを作成します：
   ```bash
   cd /Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib
   python3 test_freesasa.py
   ```

注意: 当初計画していた Rosetta の統合テスト (integration test) システムは使用しません。代わりに、独自のテスト方法で機能検証を行います。

### 8.2 パフォーマンス計測

FreeSASA 実装のパフォーマンス特性を把握しておくことが重要です：

1. **ベンチマーク計画**
   - 様々な大きさのタンパク質（100残基〜5000残基）でのSASA計算時間計測
   - Lee-RichardsとShrake-Rupleyアルゴリズムのパフォーマンス比較
   - Rosetta既存のSASA実装との速度比較

2. **メモリ使用量**
   - 各アルゴリズムのメモリ使用量プロファイリング
   - 大規模構造での挙動確認

3. **結果をドキュメント化**
   ```
   # FreeSASA パフォーマンス測定結果
   
   | PDB | 残基数 | Lee-Richards (ms) | Shrake-Rupley (ms) | Rosetta既存実装 (ms) |
   |-----|-------|------------------|-------------------|-------------------|
   | 1UBQ | 76   | XX               | XX                | XX                |
   | 4HHB | 574  | XX               | XX                | XX                |
   | ...  | ...  | ...              | ...               | ...               |
   ```

### 8.3 バージョン管理と互換性

FreeSASA の将来のバージョンアップに備えた計画：

1. **バージョン情報の保持**
   ```cpp
   // FreeSASA.hh 内に追加
   static const std::string FREESASA_VERSION = "2.1.2"; // 使用しているバージョン
   ```

2. **APIの互換性チェック**
   FreeSASA APIに変更があった場合の互換性チェックとその対処法

3. **アップグレードガイド**
   将来のFreeSASAバージョンアップグレード時の手順書

### 8.4 トラブルシューティングガイド

統合中や使用中に発生しうる問題とその対処法：

1. **よくある問題と解決法**
   - ビルドエラー
   - リンクエラー
   - 実行時エラー
   - SASA計算の問題

2. **診断ツール**
   ```cpp
   // FreeSASA.cc に追加
   void FreeSASA::diagnostics(std::ostream& out) {
     out << "FreeSASA Diagnostics:" << std::endl;
     out << "  Version: " << FREESASA_VERSION << std::endl;
     out << "  Probe radius: " << probe_radius_ << " Å" << std::endl;
     out << "  Algorithm: " << (algorithm_ == 0 ? "Lee-Richards" : "Shrake-Rupley") << std::endl;
     out << "  Resolution: " << resolution_ << std::endl;
     
     // ライブラリの状態チェック
   #ifdef USE_FREESASA
     out << "  FreeSASA library: Available" << std::endl;
     // FreeSASAライブラリの内部状態チェック
   #else
     out << "  FreeSASA library: Not available (USE_FREESASA not defined)" << std::endl;
   #endif
   }
   ```

## 9. Xcode での開発とデバッグの詳細

### 9.1 Xcode プロジェクトの設定とコード開発

Xcode を使用して FreeSASA インターフェースの開発とデバッグを効率的に行うための詳細手順：

1. **プロジェクトの生成と設定**
   ```bash
   cd /Users/nobuyasu/dev_2025/rosetta/source/xcode
   python make_project.py --extras=freesasa
   ```

2. **デバッグ設定**
   - Xcode でプロジェクトを開く
   - ナビゲータでクラスを検索して `FreeSASA.cc` を開く
   - ブレークポイントを設定する箇所：
     - `FreeSASA` コンストラクタ
     - `calculateAtomSASA` メソッド内の FreeSASA 構造体作成部分
     - `calculateResidueSASA` メソッド内の SASA 合計計算部分

3. **テストアプリケーションの設定**
   - スキーム選択: `test_freesasa`
   - 引数: `-in:file:s <PDBファイルへのパス>`
   - 作業ディレクトリ: `/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib`

4. **性能プロファイリング**
   - Xcode の Instruments を使用したプロファイリング方法
   - 特に大きなタンパク質構造の処理時間と Memory Leak のチェック

### 9.2 性能評価と検証

FreeSASA 統合の性能を評価・検証するための方法：

1. **計算精度の検証**
   - 標準的なタンパク質セット（小、中、大サイズ）に対する SASA 値の計算
   - FreeSASA のオリジナル実装との結果比較（同じ入力で同じ結果が得られることを確認）
   - 異なるアルゴリズム（Lee-Richards と Shrake-Rupley）の結果の一貫性

2. **パフォーマンス評価**
   - 計算時間の測定: 異なるサイズのタンパク質構造での処理時間
   - メモリ使用量の測定: 特に大きなタンパク質構造での挙動
   - 評価結果をテーブルとしてドキュメント化

3. **機能性の検証**
   - プローブ半径設定の効果
   - アルゴリズム選択のオプションが正しく機能すること
   - 原子レベルと残基レベルの計算結果の整合性

### 9.3 複数プラットフォーム対応

FreeSASA 統合は複数のプラットフォームで動作する必要があります：

1. **macOS での確認事項**
   - Apple Silicon と Intel プロセッサの両方でのビルド・動作確認
   - SIP (System Integrity Protection) 有効環境での動作確認

2. **Linux での確認事項**
   - 各種ディストリビューション (Ubuntu, CentOS など) での動作確認
   - ビルド依存関係の最小化

3. **プリコンパイルされたバイナリの配布**
   - プラットフォームごとにプリコンパイルされたバイナリライブラリの提供方法
   - 手動ビルドの手順書

### 9.4 プロジェクト完了判断基準

プロジェクトが正常に完了したと判断するための基準：

1. **必須要件**
   - FreeSASA インターフェースが正常にビルドできる
   - 主要な機能 (原子・残基レベルのSASA計算) が正常に動作する
   - 単体テストが通過する
   - パイロットアプリケーションが正常に動作する
   - Xcode プロジェクトが正常に生成・ビルドできる

2. **品質要件**
   - コード品質: Rosetta コーディング規約への準拠
   - パフォーマンス: 既存の実装と同等以上
   - ドキュメント: クラス・メソッドの詳細なドキュメント
   - 使用例: サンプルコードと使用方法の説明

3. **チェックリスト**
   ```
   [ ] インターフェースクラスの実装
   [ ] ビルドシステムへの統合
   [ ] 単体テストの実装
   [ ] パイロットアプリケーションの実装
   [ ] Xcode プロジェクト生成
   [ ] ドキュメント作成
   [ ] コードレビュー
   [ ] パフォーマンステスト
   [ ] プラットフォーム互換性テスト
   ```

## 10. FreeSASA インターフェースクラスの使用方法

```cpp
#include <core/scoring/sasa/FreeSASA.hh>
#include <core/pose/Pose.hh>

// FreeSASAオブジェクトを作成（デフォルトのプローブ半径 = 1.4Å = 水分子の半径相当）
core::scoring::sasa::FreeSASA sasa_calc;

// ポーズを読み込み
core::pose::Pose pose;
core::import_pose::pose_from_file(pose, "input.pdb");

// 個々の原子のSASAを計算
std::map<core::id::AtomID, core::Real> atom_sasa = sasa_calc.calculateAtomSASA(pose);

// 残基ごとのSASAを計算
std::map<core::Size, core::Real> residue_sasa = sasa_calc.calculateResidueSASA(pose);

// 特定の残基のSASAにアクセス（例: 10番目の残基）
core::Real rsd10_sasa = residue_sasa[10];

// アルゴリズムをShrake-Rupleyに変更（より高速だが近似的）
// 第1引数: 0 = Lee-Richards, 1 = Shrake-Rupley
// 第2引数: 精度（スライス数またはテストポイント数）
sasa_calc.setAlgorithm(1, 100);

// プローブ半径を変更（単位: Å）
sasa_calc.setProbeRadius(1.2);

// 新しい設定で再計算
residue_sasa = sasa_calc.calculateResidueSASA(pose);
```

## 9. テスト実行方法の詳細

FreeSASA統合のテストは以下の3段階で行います：

### 9.1 単体テスト

単体テストは `/Users/nobuyasu/dev_2025/rosetta/source/test/core/scoring/sasa/FreeSASATest.cxxtest.hh` に実装します：

```cpp
// 単体テスト例
#include <cxxtest/TestSuite.h>
#include <core/scoring/sasa/FreeSASA.hh>
#include <core/pose/Pose.hh>
#include <core/import_pose/import_pose.hh>
#include <core/conformation/Residue.hh>

class FreeSASATests : public CxxTest::TestSuite {
public:
  void test_basic_functionality() {
    // 簡単なポーズを作成してテスト
    core::pose::Pose pose;
    core::import_pose::pose_from_file(pose, "test_data/2GB1.pdb");
    
    core::scoring::sasa::FreeSASA sasa_calc;
    std::map<core::Size, core::Real> rsd_sasa = sasa_calc.calculateResidueSASA(pose);
    
    // 例：各残基のSASAが0以上であることを確認
    for (auto const & pair : rsd_sasa) {
      TS_ASSERT_LESS_THAN_EQUALS(0.0, pair.second);
    }
    
    // 例：アルゴリズム変更が機能することを確認
    sasa_calc.setAlgorithm(1, 100);  // Shrake-Rupley
    std::map<core::Size, core::Real> rsd_sasa2 = sasa_calc.calculateResidueSASA(pose);
    TS_ASSERT(!rsd_sasa2.empty());
  }
};
```

単体テストの実行：
```bash
cd /Users/nobuyasu/dev_2025/rosetta/source
./scons.py -j4 mode=release cat=core.scoring.sasa
```

### 9.2 アプリケーションテスト

`test_freesasa` アプリケーションによるテスト：

```bash
cd /Users/nobuyasu/dev_2025/rosetta/source/cmake/build_release
./bin/test_freesasa -in:file:s /Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/2GB1.pdb
```

期待される出力：
- ログに各残基のSASA値が表示される
- `/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/sasa_results.txt` に結果が保存される
- Lee-RichardsとShrake-Rupleyアルゴリズムの比較結果が表示される

### 9.3 検証テスト

FreeSASAの計算結果を既知の値と比較して精度を検証します：

1. **標準PDBファイルでの検証**
   ```bash
   cd /Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib
   # DSSP/NACCESSなどの値と比較するスクリプト
   python verify_sasa_results.py
   ```

2. **Rosetta既存SASA計算との比較**
   ```bash
   cd /Users/nobuyasu/dev_2025/rosetta/source/cmake/build_release
   ./bin/test_freesasa -in:file:s test.pdb -compare_with_legacy true
   ```

テスト結果のレポートを `/Users/nobuyasu/dev_2025/rosetta/mytest/add_freesasa_lib/TEST_REPORT.md` に記録し、統合の成功基準を満たしているか確認します。