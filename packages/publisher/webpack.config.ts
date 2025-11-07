import path from "node:path";
import HtmlWebpackPlugin from "html-webpack-plugin";
import TsconfigPathsPlugin from "tsconfig-paths-webpack-plugin";
// biome-ignore lint/style/useImportType: webpack config
import webpack from "webpack";

const SRC_PATH = path.join(__dirname, "src");

const node_env =
	process.env.NODE_ENV == "development" ? "development" : "production";
const current_env =
	process.env.NODE_ENV === undefined ? "production" : node_env;

import crypto from "node:crypto";

/**
 * The MD4 algorithm is not available anymore in Node.js 17+ (because of library SSL 3).
 * In that case, silently replace MD4 by the MD5 algorithm.
 */
try {
	crypto.createHash("md4");
} catch (_e) {
	const origCreateHash = crypto.createHash;
	crypto.createHash = (alg, opts) => {
		return origCreateHash(alg === "md4" ? "md5" : alg, opts);
	};
}

const config: webpack.Configuration = {
	entry: "./src/index.tsx",
	mode: current_env,
	output: {
		path: path.resolve(__dirname, "build"),
		filename: "index.bundle.js",
	},
	resolve: {
		extensions: [".tsx", ".ts", ".js"],
		plugins: [new TsconfigPathsPlugin({})],
	},
	devtool: "source-map",
	performance: {
		hints: false,
	},
	module: {
		strictExportPresence: true,
		rules: [
			{
				test: /\.(js|jsx|tsx|ts)$/,
				exclude: /node_modules/,
				use: {
					loader: "babel-loader",
					options: {
						presets: [
							[
								"@babel/preset-env",
								{
									shippedProposals: true,
									modules: "commonjs",
									targets: { node: "8.6.0" },
								},
							],
							["@babel/preset-react", { runtime: "automatic" }],
							"@babel/preset-typescript",
						],
					},
				},
			},
			{
				test: /\.(css|scss)$/,
				use: [
					{
						loader: "style-loader",
					},
					{
						loader: "css-loader",
					},
					{
						loader: "css-modules-typescript-loader",
					},
				],
			},
			{
				test: /\.(png|jpe?g|gif|ttf|otf|jsfl)$/i,
				use: [
					{
						loader: "file-loader",
						options: {
							esModule: false,
							useRelativePath: true,
							name: "public/[name][hash].[ext]",
						},
					},
				],
			},
		],
	},
	plugins: [
		new HtmlWebpackPlugin({
			template: path.join(SRC_PATH, "index.html"),
		}),
	],
};

export default config;
